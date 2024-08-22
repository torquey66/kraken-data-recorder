#pragma once

#include "config.hpp"
#include "metrics.hpp"
#include "refdata.hpp"
#include "session.hpp"
#include "sink.hpp"

#include <simdjson.h>

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
  using ioc_t = boost::asio::io_context;
  using ssl_context_t = boost::asio::ssl::context;

  using recv_cb_t = session_t::recv_cb_t;

  engine_t(ioc_t &ioc, ssl_context_t &ssl_context, const config_t &config,
           const sink_t &sink);

  void start_processing(const recv_cb_t &cb) { m_session.start_processing(cb); }
  bool keep_processing() const { return m_session.keep_processing(); }
  void stop_processing() { m_session.stop_processing(); }

  /** Return false to cease processing and shut down. */
  bool handle_msg(msg_t);

private:
  using doc_t = simdjson::ondemand::document;

  bool handle_instrument_msg(doc_t &);
  bool handle_instrument_snapshot(doc_t &);
  bool handle_instrument_update(doc_t &);

  bool handle_book_msg(doc_t &);
  bool handle_trade_msg(doc_t &);

  bool handle_heartbeat_msg(doc_t &);
  bool handle_pong_msg(doc_t &);

  session_t m_session;
  config_t m_config;

  simdjson::ondemand::parser m_parser;

  bool m_subscribed = false;

  req_id_t m_book_req_id = 0;
  req_id_t m_inst_req_id = 0;
  req_id_t m_trade_req_id = 0;

  sink_t m_sink;

  metrics_t m_metrics;
};

} // namespace kdr
