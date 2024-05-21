/* Copyright (C) 2024 John C. Finley - All rights reserved */

#include "config.hpp"
#include "engine.hpp"
#include "level_book.hpp"
#include "parquet.hpp"
#include "session.hpp"
#include "sink.hpp"
#include "types.hpp"

#include <boost/beast/core.hpp>
#include <boost/beast/ssl.hpp>
#include <boost/log/trivial.hpp>
#include <boost/program_options.hpp>

#include <csignal>
#include <string>

namespace po = boost::program_options;

/**
 * Maintain parquet sinks for books and trades.
 */
struct parquet_sink_t final : public krakpot::sink_t {

  parquet_sink_t(std::string parquet_dir) :
    m_book_sink{parquet_dir},
    m_trades_sink{parquet_dir} {}
  
  void accept(const krakpot::response::book_t &response) override {
    m_book_sink.accept(response);
  }

  void accept(const krakpot::response::trades_t &response) override {
    m_trades_sink.accept(response);
  }

private:
  krakpot::pq::book_sink_t m_book_sink;
  krakpot::pq::trades_sink_t m_trades_sink;
};

/**
 * Maintain sink for in-memory level book.
 */
struct level_book_sink_t final : public krakpot::sink_t {

  level_book_sink_t(krakpot::depth_t depth)
    : m_level_book{depth} {}
  
  void accept(const krakpot::response::book_t &response) override {
    m_level_book.accept(response);
  }

  void accept(const krakpot::response::trades_t &) override {
    // N/A for level books
  }

private:
  krakpot::model::level_book_t m_level_book;
};


bool shutting_down = false;

void signal_handler(const boost::system::error_code &ec, int signal_number) {
  BOOST_LOG_TRIVIAL(error) << "received signal_number: " << signal_number
                           << " error: " << ec.message() << " -- shutting down";
  shutting_down = true;
}

int main(int argc, char *argv[]) {

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
    (config_t::c_parquet_dir, po::value<std::string>()->default_value("/tmp"), "directory in which to write parquet output")
    (config_t::c_book_depth, po::value<int64_t>()->default_value(1000), "one of {10, 25, 100, 500, 1000}")
    ;
    // clang-format on

    po::variables_map vm;
    po::store(po::parse_command_line(argc, argv, desc), vm);
    po::notify(vm);
    if (vm.count("help")) {
      std::cout << desc << std::endl;
      return 1;
    }

    const auto config =
        config_t{vm[config_t::c_ping_interval_secs].as<size_t>(),
                 vm[config_t::c_kraken_host].as<std::string>(),
                 vm[config_t::c_kraken_port].as<std::string>(),
                 vm[config_t::c_parquet_dir].as<std::string>(),
                 krakpot::depth_t{vm[config_t::c_book_depth].as<int64_t>()}};

    boost::asio::ssl::context ctx{boost::asio::ssl::context::tlsv13_client};
    ctx.set_options(boost::asio::ssl::context::default_workarounds |
                    boost::asio::ssl::context::no_sslv2 |
                    boost::asio::ssl::context::no_sslv3 |
                    boost::asio::ssl::context::no_tlsv1 |
                    boost::asio::ssl::context::no_tlsv1_1);

    boost::asio::io_context ioc;
    boost::asio::signal_set signals(ioc, SIGINT);
    signals.async_wait(signal_handler);

    // !@# TODO: clean up this initialization mess
    krakpot::multisink_t::sink_vector_t sinks;
    sinks.emplace_back(std::make_unique<parquet_sink_t>(config.parquet_dir()));
    sinks.emplace_back(std::make_unique<level_book_sink_t>(config.book_depth()));
    krakpot::multisink_t multisink{sinks};

    auto session = krakpot::session_t(ioc, ctx, config);
    auto engine = krakpot::engine_t(session, config);

    const auto handle_recv =
        [&engine](krakpot::msg_t msg,
                  krakpot::session_t::yield_context_t yield) {
          try {
            return engine.handle_msg(msg, yield);
          } catch (const std::exception &ex) {
            BOOST_LOG_TRIVIAL(error) << ex.what();
            return false;
          }
        };
    session.start_processing(handle_recv);

    while (!shutting_down) {
      ioc.run_one();
    }
    BOOST_LOG_TRIVIAL(error) << "session.stop_processing()";
    session.stop_processing();
    ioc.run();
  } catch (const std::exception &ex) {
    BOOST_LOG_TRIVIAL(error) << ex.what();
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}
