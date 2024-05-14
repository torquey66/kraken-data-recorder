/* Copyright (C) 2024 John C. Finley - All rights reserved */
#pragma once

#include "types.hpp"

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
  using yield_context_t = boost::asio::yield_context;

  using recv_cb_t = std::function<bool(msg_t, yield_context_t)>;

  session_t(ioc_t &, ssl_context_t &);

  void start_processing(const recv_cb_t &);
  void stop_processing() { m_keep_processing = false; }
  void send(msg_t, yield_context_t);
  void send(const std::string &msg, yield_context_t yield) {
    send(std::string_view(msg.data(), msg.size()), yield);
  }

private:
  void handshake(std::string host, std::string port, yield_context_t);
  void ping(yield_context_t);
  void process(const recv_cb_t &, yield_context_t);

  ioc_t &m_ioc;
  websocket_t m_ws;

  bool m_keep_processing = false;
  req_id_t m_req_id = 0;
};

} // namespace krakpot
