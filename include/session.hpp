#pragma once

#include "config.hpp"
#include "types.hpp"

#include <boost/asio.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/ssl.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/beast/websocket/ssl.hpp>

#include <functional>
#include <string>
#include <string_view>

namespace kdr {

/**
 * session_t manages our websocket connection to the
 * venue. Specifically, it:
 *
 * - establishes the websocket connection and performs an SSL handshake
 * - initiates a periodic ping operation
 * - invokes a callback when it receives messages from the venue
 * - enables client code to send messages to the venue
 */
struct session_t final {
  using ioc_t = boost::asio::io_context;
  using ssl_context_t = boost::asio::ssl::context;
  using websocket_t = boost::beast::websocket::stream<
      boost::beast::ssl_stream<boost::beast::tcp_stream>>;

  using recv_cb_t = std::function<bool(msg_t)>;

  session_t(ioc_t &ioc, ssl_context_t &ssl_context, const config_t &config);

  bool keep_processing() const { return m_keep_processing; }
  void start_processing(const recv_cb_t &);
  void stop_processing() {
    m_keep_processing = false;
    m_ping_timer.cancel();
  }
  void send(msg_t);
  void send(const std::string &msg) {
    send(std::string_view(msg.data(), msg.size()));
  }

private:
  using error_code = boost::beast::error_code;
  using resolver = boost::asio::ip::tcp::resolver;

  void fail(boost::beast::error_code, char const *);

  void on_resolve(error_code, resolver::results_type);
  void on_connect(error_code, resolver::results_type::endpoint_type);
  void on_ssl_handshake(error_code);
  void on_handshake(error_code);
  void on_ping_timer(error_code);
  void on_write(error_code, size_t);
  void on_read(error_code, size_t);
  void on_close(error_code);

  ioc_t &m_ioc;
  resolver m_resolver;
  websocket_t m_ws;
  boost::asio::deadline_timer m_ping_timer;
  config_t m_config;

  bool m_keep_processing = false;
  recv_cb_t m_handle_recv = [](msg_t) { return true; };
  req_id_t m_req_id = 0;

  boost::beast::flat_buffer m_read_buffer;
  std::string m_read_msg_str;
};

} // namespace kdr
