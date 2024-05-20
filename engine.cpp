/* Copyright (C) 2024 John C. Finley - All rights reserved */

#include "engine.hpp"

#include "config.hpp"
#include "constants.hpp"
#include "requests.hpp"
#include "responses.hpp"

#include <boost/log/trivial.hpp>
#include <simdjson.h>

#include <algorithm>
#include <vector>

namespace krakpot {

engine_t::engine_t(session_t &session, const config_t &config)
    : m_session{session}, m_config{config}, m_level_book{config.book_depth()},
      m_book_sink{config.parquet_dir()}, m_trades_sink{config.parquet_dir()} {}

bool engine_t::handle_msg(msg_t msg, yield_context_t yield) {

  m_metrics.accept(msg);

  try {
    simdjson::padded_string padded_msg{msg};
    simdjson::ondemand::document doc = m_parser.iterate(padded_msg);

    auto buffer = std::string_view{};
    if (doc[c_response_channel].get(buffer) == simdjson::SUCCESS) {
      if (buffer == c_channel_instrument) {
        return handle_instrument_msg(doc, yield);
      }
      if (buffer == c_channel_book) {
        return handle_book_msg(doc, yield);
      }
      if (buffer == c_channel_trade) {
        return handle_trade_msg(doc, yield);
      }
      if (buffer == c_channel_heartbeat) {
        return handle_heartbeat_msg(doc, yield);
      }
    } else if (doc[c_response_method].get(buffer) == simdjson::SUCCESS) {
      if (buffer == c_method_pong) {
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
  if (doc[c_header_type].get(buffer) != simdjson::SUCCESS) {
    BOOST_LOG_TRIVIAL(error)
        << __FUNCTION__ << ": missing 'type' " << simdjson::to_json_string(doc);
    return false;
  }

  if (buffer == c_instrument_snapshot) {
    return handle_instrument_snapshot(doc, yield);
  } else if (buffer == c_instrument_update) {
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

  const request::subscribe_book_t subscribe_book{
      ++m_book_req_id, m_config.book_depth(), true, symbols};
  m_session.send(subscribe_book.str(), yield);

  const request::subscribe_trade_t subscribe_trade{++m_trade_req_id, true,
                                                   symbols};
  m_session.send(subscribe_trade.str(), yield);

  return true;
}

bool engine_t::handle_instrument_update(doc_t &doc, yield_context_t) {

  BOOST_LOG_TRIVIAL(debug) << __FUNCTION__ << " TODO: handle updates -- doc: "
                           << simdjson::to_json_string(doc);
  return true;
}

bool engine_t::handle_book_msg(doc_t &doc, yield_context_t yield) {
  auto buffer = std::string_view{};
  if (doc[c_header_type].get(buffer) != simdjson::SUCCESS) {
    BOOST_LOG_TRIVIAL(error)
        << __FUNCTION__ << ": missing 'type' " << simdjson::to_json_string(doc);
    return false;
  }

  if (buffer == c_book_type_snapshot) {
    return handle_book_snapshot(doc, yield);
  } else if (buffer == c_book_type_update) {
    return handle_book_update(doc, yield);
  }

  BOOST_LOG_TRIVIAL(error) << __FUNCTION__ << ": unknown 'type' " << buffer;
  return false;
}

bool engine_t::handle_book_snapshot(doc_t &doc, yield_context_t) {
  const auto response = response::book_t::from_json(doc);
  m_book_sink.accept(response);
  m_level_book.accept(response);
  return true;
}

bool engine_t::handle_book_update(doc_t &doc, yield_context_t) {
  const auto response = response::book_t::from_json(doc);
  m_book_sink.accept(response);
  m_level_book.accept(response);
  return true;
}

bool engine_t::handle_trade_msg(doc_t &doc, yield_context_t) {
  const auto response = response::trades_t::from_json(doc);
  m_trades_sink.accept(response);
  return true;
}

bool engine_t::handle_heartbeat_msg(doc_t &, yield_context_t) {
  // !@# TODO: track a stat on time between heartbeats?
  return true;
}

bool engine_t::handle_pong_msg(doc_t & /*doc*/, yield_context_t yield) {
  // !@# TODO: track ping/pong latency
  if (!m_subscribed) {
    const request::subscribe_instrument_t subscribe_inst{++m_inst_req_id};
    m_session.send(subscribe_inst.str(), yield);
    m_subscribed = true;
  }
  BOOST_LOG_TRIVIAL(info) << m_metrics.str();
  return true;
}

} // namespace krakpot
