/* Copyright (C) 2024 John C. Finley - All rights reserved */
#pragma once

#include "level_book.hpp"
#include "parquet.hpp"
#include "session.hpp"

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

  using msg_t = session_t::msg_t;
  using yield_context_t = session_t::yield_context_t;

  engine_t(session_t &);

  /** Return false to cease processing and shut down. */
  bool handle_msg(msg_t, yield_context_t);

private:
  using doc_t = simdjson::ondemand::document;

  bool handle_instrument_msg(doc_t &, yield_context_t);
  bool handle_instrument_snapshot(doc_t &, yield_context_t);
  bool handle_instrument_update(doc_t &, yield_context_t);

  bool handle_book_msg(doc_t &, yield_context_t);
  bool handle_book_snapshot(doc_t &, yield_context_t);
  bool handle_book_update(doc_t &, yield_context_t);

  bool handle_trade_msg(doc_t &, yield_context_t);

  bool handle_heartbeat_msg(doc_t &, yield_context_t);
  bool handle_pong_msg(doc_t &, yield_context_t);

  session_t &m_session;

  simdjson::ondemand::parser m_parser;

  bool m_subscribed = false;

  req_id_t m_book_req_id = 0;
  req_id_t m_inst_req_id = 0;
  req_id_t m_trade_req_id = 0;

  model::level_book_t m_level_book;
  pq::book_sink_t m_book_sink;
  pq::trades_sink_t m_trades_sink;
};

} // namespace krakpot
