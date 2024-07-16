/* Copyright (C) 2024 John C. Finley - All rights reserved */
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
namespace krakpot {

struct engine_t final {
  engine_t(session_t&, const config_t&, const sink_t&);

  /** Return false to cease processing and shut down. */
  bool handle_msg(msg_t);

 private:
  using doc_t = simdjson::ondemand::document;

  bool handle_instrument_msg(std::string_view);
  bool handle_instrument_snapshot(const response::instrument_t&);
  bool handle_instrument_update(const response::instrument_t&);

  bool handle_book_msg(doc_t&);
  bool handle_trade_msg(doc_t&);

  bool handle_heartbeat_msg(doc_t&);
  bool handle_pong_msg(doc_t&);

  session_t& m_session;
  config_t m_config;

  simdjson::ondemand::parser m_parser;

  bool m_subscribed = false;

  req_id_t m_book_req_id = 0;
  req_id_t m_inst_req_id = 0;
  req_id_t m_trade_req_id = 0;

  sink_t m_sink;

  metrics_t m_metrics;
};

}  // namespace krakpot
