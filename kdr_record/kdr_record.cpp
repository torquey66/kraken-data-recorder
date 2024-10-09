#include "assets_sink.hpp"
#include "book_sink.hpp"
#include "config.hpp"
#include "depth.hpp"
#include "engine.hpp"
#include "level_book.hpp"
#include "pairs_sink.hpp"
#include "shmem_sink.hpp"
#include "sink.hpp"
#include "trade_sink.hpp"
#include "types.hpp"

#include <boost/beast/core.hpp>
#include <boost/beast/ssl.hpp>
#include <boost/log/core.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/trivial.hpp>
#include <boost/program_options.hpp>

#include <csignal>
#include <string>

namespace po = boost::program_options;

bool shutting_down = false;

void signal_handler(const boost::system::error_code &ec, int signal_number) {
  BOOST_LOG_TRIVIAL(error) << "received signal_number: " << signal_number
                           << " error: " << ec.message() << " -- shutting down";
  shutting_down = true;
}

int main(int argc, char *argv[]) {

  // !@# TODO: add program option to engage debug level...
  boost::log::core::get()->set_filter(boost::log::trivial::severity >=
                                      boost::log::trivial::info);

  po::options_description desc(
      "Subscribe to Kraken and serialize book/trade data");

  using kdr::config_t;

  std::vector<std::string> pairs_filter_vector;

  // clang-format off
    desc.add_options()
      ("help", "display program options")
      (config_t::c_ping_interval_secs.data(), po::value<size_t>()->default_value(30), "ping/pong delay")
      (config_t::c_kraken_host.data(), po::value<std::string>()->default_value("ws.kraken.com"), "Kraken websocket host")
      (config_t::c_kraken_port.data(), po::value<std::string>()->default_value("443"), "Kraken websocket port")
      (config_t::c_pair_filter.data(), po::value<std::vector<std::string>>(&pairs_filter_vector)->multitoken(),
       "pairs to record or empty to record all")
       (config_t::c_parquet_dir.data(), po::value<std::string>()->default_value("/tmp"), "directory in which to write parquet output")
      (config_t::c_book_depth.data(), po::value<int64_t>()->default_value(1000), "one of {10, 25, 100, 500, 1000}")
      (config_t::c_capture_book.data(), po::value<bool>()->default_value(true), "subscribe to and record level book")
      (config_t::c_capture_trades.data(), po::value<bool>()->default_value(true), "subscribe to and record trades")
      (config_t::c_enable_shmem.data(), po::value<bool>()->default_value(false), "enable shared memory sink")
    ;
  // clang-format on

  po::variables_map vm;
  po::store(po::parse_command_line(argc, argv, desc), vm);
  po::notify(vm);
  if (vm.count("help")) {
    std::cout << desc << std::endl;
    return 1;
  }

  const config_t::symbol_filter_t pair_filter{pairs_filter_vector.begin(),
                                              pairs_filter_vector.end()};

  const auto config = config_t{
      vm[config_t::c_ping_interval_secs.data()].as<size_t>(),
      vm[config_t::c_kraken_host.data()].as<std::string>(),
      vm[config_t::c_kraken_port.data()].as<std::string>(),
      pair_filter,
      vm[config_t::c_parquet_dir.data()].as<std::string>(),
      kdr::model::depth_t{vm[config_t::c_book_depth.data()].as<int64_t>()},
      vm[config_t::c_capture_book.data()].as<bool>(),
      vm[config_t::c_capture_trades.data()].as<bool>(),
      vm[config_t::c_enable_shmem.data()].as<bool>()};

  BOOST_LOG_TRIVIAL(info) << kdr::c_license;
  BOOST_LOG_TRIVIAL(info) << "starting up with config: " << config.str();

  boost::asio::ssl::context ctx{boost::asio::ssl::context::tlsv13_client};
  ctx.set_options(boost::asio::ssl::context::default_workarounds |
                  boost::asio::ssl::context::no_sslv2 |
                  boost::asio::ssl::context::no_sslv3 |
                  boost::asio::ssl::context::no_tlsv1 |
                  boost::asio::ssl::context::no_tlsv1_1);

  const auto now = kdr::timestamp_t::now().micros();

  kdr::pq::assets_sink_t assets_sink{config.parquet_dir(), now};
  kdr::pq::pairs_sink_t pairs_sink{config.parquet_dir(), now};
  kdr::pq::book_sink_t book_sink{config.parquet_dir(), now,
                                 config.book_depth()};
  kdr::pq::trades_sink_t trades_sink{config.parquet_dir(), now};

  kdr::model::level_book_t level_book{config.book_depth()};
  kdr::model::refdata_t refdata;

  kdr::shmem::shmem_sink_t shmem_sink;

  using shmem_accept_instrument_t =
      std::function<void(const kdr::response::instrument_t &)>;
  const shmem_accept_instrument_t noop_shmem_accept_instrument{
      [](const kdr::response::instrument_t &) {}};
  shmem_accept_instrument_t shmem_accept_instrument{
      config.enable_shmem()
          ? [&shmem_sink](const kdr::response::instrument_t
                              &response) { shmem_sink.accept(response); }
          : noop_shmem_accept_instrument};

  using shmem_accept_book_t =
      std::function<void(const kdr::response::book_t &)>;
  const shmem_accept_book_t noop_shmem_accept_book{
      [](const kdr::response::book_t &) {}};
  shmem_accept_book_t shmem_accept_book{
      config.enable_shmem()
          ? [&shmem_sink, &level_book](
                const kdr::response::book_t
                    &response) { shmem_sink.accept(response, level_book); }
          : noop_shmem_accept_book};

  const auto accept_instrument =
      [&level_book, &assets_sink, &pairs_sink, &refdata,
       &shmem_accept_instrument](const kdr::response::instrument_t &response) {
        assets_sink.accept(response.header(), response.assets());
        pairs_sink.accept(response.header(), response.pairs());
        refdata.accept(response);
        for (const auto &pair : response.pairs()) {
          level_book.accept(pair);
          BOOST_LOG_TRIVIAL(debug)
              << "created/updated book for symbol: " << pair.symbol();
        }
        shmem_accept_instrument(response);
      };

  const auto noop_accept_book = [](const kdr::response::book_t &) {};
  const auto accept_book =
      [&book_sink, &level_book, &refdata,
       &shmem_accept_book](const kdr::response::book_t &response) {
        book_sink.accept(response, refdata);
        level_book.accept(response);
        shmem_accept_book(response);
      };

  const auto noop_accept_trades = [](const kdr::response::trades_t &) {};
  const auto accept_trades =
      [&trades_sink, &refdata](const kdr::response::trades_t &response) {
        trades_sink.accept(response, refdata);
      };
  const kdr::sink_t sink{
      accept_instrument,
      config.capture_book() ? kdr::sink_t::accept_book_t{accept_book}
                            : kdr::sink_t::accept_book_t{noop_accept_book},
      config.capture_trades()
          ? kdr::sink_t::accept_trades_t{accept_trades}
          : kdr::sink_t::accept_trades_t{noop_accept_trades}};

  auto engine = kdr::engine_t(ctx, config, sink);
  const auto handle_recv = [&engine](kdr::msg_t msg) {
    try {
      return engine.handle_msg(msg);
    } catch (const std::exception &ex) {
      BOOST_LOG_TRIVIAL(error) << "handle_recv: " << ex.what();
      return false;
    }
  };

  auto &ioc = engine.session().ioc();
  boost::asio::signal_set signals(ioc, SIGINT);
  signals.async_wait(signal_handler);

  engine.start_processing(handle_recv);

  while (!shutting_down && engine.keep_processing()) {
    ioc.run_one();
  }
  BOOST_LOG_TRIVIAL(info) << "session.stop_processing()";
  engine.stop_processing();

  return EXIT_SUCCESS;
}
