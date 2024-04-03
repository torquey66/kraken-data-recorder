/* Copyright (C) 2024 John C. Finley - All rights reserved */

#include "cert.hpp"
#include "engine.hpp"
#include "session.hpp"

#include <boost/beast/core.hpp>
#include <boost/beast/ssl.hpp>
#include <boost/log/trivial.hpp>

#include <string>
#include <string_view>

int main(int argc, char **argv) {
  boost::asio::io_context ioc;
  boost::asio::ssl::context ctx{boost::asio::ssl::context::tlsv13_client};
  krakpot::load_root_certificates(ctx);

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

  ioc.run();

  return EXIT_SUCCESS;
}
