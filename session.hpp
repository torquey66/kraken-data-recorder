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

#include <functional>
#include <string>
#include <string_view>

namespace krakpot {

struct session_t final {

  using ioc_t = boost::asio::io_context;
  using ssl_context_t = boost::asio::ssl::context;
  using websocket_t = boost::beast::websocket::stream<
      boost::beast::ssl_stream<boost::beast::tcp_stream>>;
  using yield_context_t = boost::asio::yield_context;

  using msg_t = std::string_view;
  using recv_cb_t = std::function<bool(msg_t)>;

  session_t(ioc_t &, ssl_context_t &);

  void start_processing(const recv_cb_t &);
  void send(msg_t, yield_context_t);
  void send(const std::string &msg, yield_context_t yield) {
    send(std::string_view(msg.data(), msg.size()), yield);
  }

private:
  void handshake(std::string host, std::string port, yield_context_t);
  void ping(yield_context_t);
  void process(const recv_cb_t &, yield_context_t);
  void subscribe(yield_context_t);

  ioc_t &m_ioc;
  ssl_context_t &m_ssl_context;
  websocket_t m_ws;

  req_id_t m_req_id = 0;
};

} // namespace krakpot
