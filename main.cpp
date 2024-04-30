/* Copyright (C) 2024 John C. Finley - All rights reserved */

#include "engine.hpp"
#include "session.hpp"

#include <boost/beast/core.hpp>
#include <boost/beast/ssl.hpp>
#include <boost/log/trivial.hpp>

#include <csignal>
#include <string>

bool shutting_down = false;

void signal_handler(const boost::system::error_code &ec, int signal_number) {
  BOOST_LOG_TRIVIAL(error) << "received signal_number: " << signal_number
                           << " error: " << ec.message() << " -- shutting down";
  shutting_down = true;
}

int main(int, char **) {
  boost::asio::ssl::context ctx{boost::asio::ssl::context::tlsv13_client};
  ctx.set_options(boost::asio::ssl::context::default_workarounds |
                  boost::asio::ssl::context::no_sslv2 |
                  boost::asio::ssl::context::no_sslv3 |
                  boost::asio::ssl::context::no_tlsv1 |
                  boost::asio::ssl::context::no_tlsv1_1);

  try {
    boost::asio::io_context ioc;
    boost::asio::signal_set signals(ioc, SIGINT);
    signals.async_wait(signal_handler);

    auto session = krakpot::session_t(ioc, ctx);
    auto engine = krakpot::engine_t(session);

    const auto handle_recv =
        [&engine](krakpot::session_t::msg_t msg,
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
