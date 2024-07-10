/* Copyright (C) 2024 John C. Finley - All rights reserved */

#include "assets_sink.hpp"
#include "book_sink.hpp"
#include "config.hpp"
#include "engine.hpp"
#include "level_book.hpp"
#include "pairs_sink.hpp"
#include "session.hpp"
#include "sink.hpp"
#include "trade_sink.hpp"
#include "types.hpp"

#include <boost/beast/core.hpp>
#include <boost/beast/ssl.hpp>
#include <boost/log/trivial.hpp>
#include <boost/program_options.hpp>

#include <csignal>
#include <string>

namespace po = boost::program_options;

bool shutting_down = false;

void signal_handler(const boost::system::error_code& ec, int signal_number) {
  BOOST_LOG_TRIVIAL(error) << "received signal_number: " << signal_number
                           << " error: " << ec.message() << " -- shutting down";
  shutting_down = true;
}

int main(int argc, char* argv[]) {
  boost::asio::io_context ioc;
  try {
    po::options_description desc(
        "Subscribe to Kraken and serialize book/trade data");

    using krakpot::config_t;

    // clang-format off
    desc.add_options()
      ("help", "display program options")
      (config_t::c_ping_interval_secs, po::value<size_t>()->default_value(30), "ping/pong delay")
      (config_t::c_kraken_host, po::value<std::string>()->default_value("ws.kraken.com"), "Kraken websocket host")
      (config_t::c_kraken_port, po::value<std::string>()->default_value("443"), "Kraken websocket port")
      (config_t::c_pair_filter, po::value<std::string>()->default_value("[]"), "explicit list of pairs to subscribe to as json string")
      (config_t::c_parquet_dir, po::value<std::string>()->default_value("/tmp"), "directory in which to write parquet output")
      (config_t::c_book_depth, po::value<int64_t>()->default_value(1000), "one of {10, 25, 100, 500, 1000}")
      (config_t::c_capture_book, po::value<bool>()->default_value(true), "subscribe to and record level book")
      (config_t::c_capture_trades, po::value<bool>()->default_value(true), "subscribe to and record trades")
    ;
    // clang-format on

    po::variables_map vm;
    po::store(po::parse_command_line(argc, argv, desc), vm);
    po::notify(vm);
    if (vm.count("help")) {
      std::cout << desc << std::endl;
      return 1;
    }

    const auto pair_filter_json =
        nlohmann::json::parse(vm[config_t::c_pair_filter].as<std::string>());

    const auto config =
        config_t{vm[config_t::c_ping_interval_secs].as<size_t>(),
                 vm[config_t::c_kraken_host].as<std::string>(),
                 vm[config_t::c_kraken_port].as<std::string>(),
                 pair_filter_json.get<config_t::symbol_filter_t>(),
                 vm[config_t::c_parquet_dir].as<std::string>(),
                 krakpot::depth_t{vm[config_t::c_book_depth].as<int64_t>()},
                 vm[config_t::c_capture_book].as<bool>(),
                 vm[config_t::c_capture_trades].as<bool>()};

    BOOST_LOG_TRIVIAL(info)
        << "starting up with config: " << config.to_json().dump(3);

    boost::asio::ssl::context ctx{boost::asio::ssl::context::tlsv13_client};
    ctx.set_options(boost::asio::ssl::context::default_workarounds |
                    boost::asio::ssl::context::no_sslv2 |
                    boost::asio::ssl::context::no_sslv3 |
                    boost::asio::ssl::context::no_tlsv1 |
                    boost::asio::ssl::context::no_tlsv1_1);

    boost::asio::signal_set signals(ioc, SIGINT);
    signals.async_wait(signal_handler);

    const auto now = krakpot::timestamp_t::now().micros();

    krakpot::pq::assets_sink_t assets_sink{config.parquet_dir(), now};
    krakpot::pq::pairs_sink_t pairs_sink{config.parquet_dir(), now};
    krakpot::pq::book_sink_t book_sink{config.parquet_dir(), now,
                                       config.book_depth()};
    krakpot::pq::trades_sink_t trades_sink{config.parquet_dir(), now};

    krakpot::model::level_book_t level_book{config.book_depth()};
    krakpot::model::refdata_t refdata;

    const auto accept_instrument =
        [&level_book, &assets_sink, &pairs_sink,
         &refdata](const krakpot::response::instrument_t& response) {
          assets_sink.accept(response.header(), response.assets());
          pairs_sink.accept(response.header(), response.pairs());
          refdata.accept(response);
          for (const auto& pair : response.pairs()) {
            level_book.accept(pair);
            BOOST_LOG_TRIVIAL(debug)
                << "created/updated book for symbol: " << pair.symbol();
          }
        };

    // !@# TODO: replace use of level book with refdata
    const auto noop_accept_book = [](const krakpot::response::book_t&) {};
    const auto accept_book = [&book_sink, &level_book, &refdata](
                                 const krakpot::response::book_t& response) {
      book_sink.accept(response, refdata);
      level_book.accept(response);
    };

    const auto noop_accept_trades = [](const krakpot::response::trades_t&) {};
    const auto accept_trades =
        [&trades_sink, &refdata](const krakpot::response::trades_t& response) {
          trades_sink.accept(response, refdata);
        };
    const krakpot::sink_t sink{
        accept_instrument,
        config.capture_book()
            ? krakpot::sink_t::accept_book_t{accept_book}
            : krakpot::sink_t::accept_book_t{noop_accept_book},
        config.capture_trades()
            ? krakpot::sink_t::accept_trades_t{accept_trades}
            : krakpot::sink_t::accept_trades_t{noop_accept_trades}};

    auto session = krakpot::session_t(ioc, ctx, config);
    auto engine = krakpot::engine_t(session, config, sink);

    const auto handle_recv = [&engine](krakpot::msg_t msg) {
      try {
        return engine.handle_msg(msg);
      } catch (const std::exception& ex) {
        BOOST_LOG_TRIVIAL(error) << ex.what();
        return false;
      }
    };
    session.start_processing(handle_recv);

    while (!shutting_down && session.keep_processing()) {
      ioc.run_one();
    }
    BOOST_LOG_TRIVIAL(error) << "session.stop_processing()";
    session.stop_processing();
  } catch (const std::exception& ex) {
    ioc.stop();
    BOOST_LOG_TRIVIAL(error) << ex.what();
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}
