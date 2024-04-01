#pragma once

#define BOOST_ASIO_DISABLE_BOOST_COROUTINE

#include "processor.hpp"
#include "requests.hpp"

#include <boost/asio.hpp>
#include <boost/asio/spawn.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/ssl.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/beast/websocket/ssl.hpp>

#include <string>

namespace krakpot {

struct session_t final {

  using ioc_t = boost::asio::io_context;
  using ssl_context_t = boost::asio::ssl::context;
  using websocket_t = boost::beast::websocket::stream<
      boost::beast::ssl_stream<boost::beast::tcp_stream>>;
  using yield_context_t = boost::asio::yield_context;

  session_t(ioc_t &, ssl_context_t &);

  void start_processing();

private:
  void handshake(std::string host, std::string port, yield_context_t);
  void heartbeat(yield_context_t);
  void process(yield_context_t);
  void subscribe(yield_context_t);

  ioc_t &m_ioc;
  ssl_context_t &m_ssl_context;
  websocket_t m_ws;

  request::req_id_t m_req_id = 0;
  processor_t m_processor;
};

} // namespace krakpot
