#pragma once

#include "config.hpp"
#include "metrics.hpp"
#include "refdata.hpp"
#include "session.hpp"
#include "sink.hpp"

#include <simdjson.h>

#include <queue>

/**
 * engine_t is our central dispatcher for messages received from the
 * venue. It also tracks application state, which currently consists
 * of whether or not we have subscribed to the pairs in the initial
 * instrument snapshot.
 *
 * As functionality expands, this will be broken into submodules.
 */
namespace kdr {

struct engine_t final {
  using ssl_context_t = boost::asio::ssl::context;

  using recv_cb_t = session_t::recv_cb_t;

  engine_t(ssl_context_t &ssl_context, const config_t &config,
           const sink_t &sink);

  const session_t &session() const { return m_session; }
  session_t &session() { return m_session; }

  void start_processing(const recv_cb_t &recv_cb);
  bool keep_processing() const { return m_session.keep_processing(); }
  void stop_processing() {
    m_metrics_timer.cancel();
    m_process_timer.cancel();
    m_session.stop_processing();
  }

  /** Return false to cease processing and shut down. */
  bool handle_msg(msg_t);

private:
  static constexpr auto c_metrics_interval_secs = 10;
  static constexpr auto c_process_interval_micros = 30;
  static constexpr size_t c_process_batch_size = 64;

  using doc_t = simdjson::ondemand::document;
  using error_code = boost::beast::error_code;

  bool handle_instrument_msg(doc_t &);
  bool handle_instrument_snapshot(doc_t &);
  bool handle_instrument_update(doc_t &);

  bool handle_book_msg(doc_t &);
  bool handle_trade_msg(doc_t &);

  bool handle_heartbeat_msg(doc_t &);
  bool handle_pong_msg(doc_t &);

  void on_metrics_timer(error_code ec);
  void on_process_timer(error_code ec);

  session_t m_session;
  config_t m_config;

  simdjson::ondemand::parser m_parser;

  bool m_subscribed = false;

  req_id_t m_book_req_id = 0;
  req_id_t m_inst_req_id = 0;
  req_id_t m_trade_req_id = 0;

  boost::asio::deadline_timer m_metrics_timer;
  boost::asio::steady_timer m_process_timer;
  std::queue<response::book_t> m_book_responses;

  sink_t m_sink;

  metrics_t m_metrics;
};

} // namespace kdr
