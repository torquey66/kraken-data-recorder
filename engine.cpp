/* Copyright (C) 2024 John C. Finley - All rights reserved */

#include "engine.hpp"

#include "requests.hpp"
#include "responses.hpp"

#include <boost/log/trivial.hpp>
#include <simdjson.h>

#include <algorithm>
#include <vector>

namespace krakpot {

engine_t::engine_t(session_t &session) : m_session(session) {}

bool engine_t::handle_msg(msg_t msg, yield_context_t yield) {

  try {
    simdjson::padded_string padded_msg{msg};
    simdjson::ondemand::document doc = m_parser.iterate(padded_msg);

    auto buffer = std::string_view{};
    if (doc["channel"].get(buffer) == simdjson::SUCCESS) {
      if (buffer == "instrument") {
        return handle_instrument_msg(doc, yield);
      }
      if (buffer == "book") {
        return handle_book_msg(doc, yield);
      }
      if (buffer == "heartbeat") {
        return handle_heartbeat_msg(doc, yield);
      }
    } else if (doc["method"].get(buffer) == simdjson::SUCCESS) {
      if (buffer == "pong") {
        return handle_pong_msg(doc, yield);
      }
      // !@# TODO: ultimately we will want to crack open 'subscribe'
      // !responses and handle those which report failures.
      BOOST_LOG_TRIVIAL(debug)
          << __FUNCTION__ << ": " << simdjson::to_json_string(doc);
    } else {
      BOOST_LOG_TRIVIAL(warning)
          << __FUNCTION__
          << ": unexpected message: " << simdjson::to_json_string(doc);
    }

  } catch (const std::exception &ex) {
    BOOST_LOG_TRIVIAL(error) << __FUNCTION__ << ": " << ex.what();
    return false;
  }

  return true;
}

bool engine_t::handle_instrument_msg(doc_t &doc, yield_context_t yield) {

  auto buffer = std::string_view{};
  if (doc["type"].get(buffer) != simdjson::SUCCESS) {
    BOOST_LOG_TRIVIAL(error)
        << __FUNCTION__ << ": missing 'type' " << simdjson::to_json_string(doc);
    return false;
  }

  if (buffer == "snapshot") {
    return handle_instrument_snapshot(doc, yield);
  } else if (buffer == "update") {
    return handle_instrument_update(doc, yield);
  }

  BOOST_LOG_TRIVIAL(error) << __FUNCTION__ << ": unknown 'type' " << buffer;
  return false;
}

bool engine_t::handle_instrument_snapshot(doc_t &doc, yield_context_t yield) {

  const auto response = response::instrument_t::from_json(doc);

  const auto &pairs = response.pairs();
  auto symbols = std::vector<std::string>{};
  std::transform(pairs.begin(), pairs.end(), std::back_inserter(symbols),
                 [](const auto &pair) { return pair.symbol(); });

  // !@# TODO: make depth configurable in some fashion
  const request::subscribe_book_t subscribe_book{
      ++m_book_req_id, request::subscribe_book_t::e_1000, true, symbols};
  m_session.send(subscribe_book.str(), yield);

  return true;
}

bool engine_t::handle_instrument_update(doc_t &doc, yield_context_t) {

  BOOST_LOG_TRIVIAL(debug) << __FUNCTION__ << " TODO: handle updates -- doc: "
                           << simdjson::to_json_string(doc);
  return true;
}

bool engine_t::handle_book_msg(doc_t &doc, yield_context_t yield) {
  auto buffer = std::string_view{};
  if (doc["type"].get(buffer) != simdjson::SUCCESS) {
    BOOST_LOG_TRIVIAL(error)
        << __FUNCTION__ << ": missing 'type' " << simdjson::to_json_string(doc);
    return false;
  }

  if (buffer == "snapshot") {
    return handle_book_snapshot(doc, yield);
  } else if (buffer == "update") {
    return handle_book_update(doc, yield);
  }

  BOOST_LOG_TRIVIAL(error) << __FUNCTION__ << ": unknown 'type' " << buffer;
  return false;
}

bool engine_t::handle_book_snapshot(doc_t &doc, yield_context_t) {
  const auto response = response::book_t::from_json(doc);
  BOOST_LOG_TRIVIAL(debug) << __FUNCTION__ << response.str();
  return true;
}

bool engine_t::handle_book_update(doc_t &doc, yield_context_t) {
  const auto response = response::book_t::from_json(doc);
  BOOST_LOG_TRIVIAL(debug) << __FUNCTION__ << response.str();
  return true;
}

bool engine_t::handle_heartbeat_msg(doc_t &doc, yield_context_t) {
  // !@# TODO: track a stat on time between heartbeats?
  return true;
}

bool engine_t::handle_pong_msg(doc_t &, yield_context_t yield) {
  if (!m_subscribed) {
    const request::subscribe_instrument_t subscribe_inst{++m_inst_req_id};
    m_session.send(subscribe_inst.str(), yield);
    m_subscribed = true;
  }
  return true;
}

} // namespace krakpot
