/* Copyright (C) 2024 John C. Finley - All rights reserved */

#include "engine.hpp"

#include "constants.hpp"
#include "requests.hpp"
#include "responses.hpp"

#include <boost/log/trivial.hpp>
#include <simdjson.h>

#include <algorithm>
#include <vector>

namespace krakpot {

engine_t::engine_t(session_t &session, const config_t &config,
                   const sink_t &sink)
    : m_session{session}, m_config{config}, m_sink(sink) {}

bool engine_t::handle_msg(msg_t msg) {
  m_metrics.accept(msg);

  try {
    simdjson::padded_string padded_msg{msg};
    simdjson::ondemand::document doc = m_parser.iterate(padded_msg);

    auto buffer = std::string_view{};
    if (doc[c_response_channel].get(buffer) == simdjson::SUCCESS) {
      if (buffer == c_channel_instrument) {
        return handle_instrument_msg(msg);
      }
      if (buffer == c_channel_book) {
        return handle_book_msg(doc);
      }
      if (buffer == c_channel_trade) {
        return handle_trade_msg(doc);
      }
      if (buffer == c_channel_heartbeat) {
        return handle_heartbeat_msg(doc);
      }
    } else if (doc[c_response_method].get(buffer) == simdjson::SUCCESS) {
      if (buffer == c_method_pong) {
        return handle_pong_msg(doc);
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

  } catch (const std::exception& ex) {
    BOOST_LOG_TRIVIAL(error)
        << __FUNCTION__ << ": " << ex.what() << " msg: " << msg;
    return false;
  }

  return true;
}

bool engine_t::handle_instrument_msg(std::string_view msg) {
  try {
    const boost::json::object doc = boost::json::parse(msg).as_object();
    const auto instrument = response::instrument_t::from_json_obj(doc);
    if (instrument.header().type() == c_instrument_snapshot) {
      return handle_instrument_snapshot(instrument);
    }
    if (instrument.header().type() == c_instrument_update) {
      return handle_instrument_update(instrument);
    }
    BOOST_LOG_TRIVIAL(error)
        << __FUNCTION__ << ": unknown 'type' " << instrument.header().type();
    ;
    return false;
  } catch (const std::exception& ex) {
    BOOST_LOG_TRIVIAL(error)
        << __FUNCTION__ << ": " << ex.what() << " msg: " << msg;
    return false;
  }
  return true;
}

bool engine_t::handle_instrument_snapshot(
    const response::instrument_t& instrument) {
  m_sink.accept(instrument);

  const auto& pairs = instrument.pairs();
  auto symbols = std::vector<std::string>{};
  std::transform(pairs.begin(), pairs.end(), std::back_inserter(symbols),
                 [](const auto& pair) { return pair.symbol(); });

  const auto& pair_filter = m_config.pair_filter();
  if (!pair_filter.empty()) {
    const auto end =
        std::remove_if(symbols.begin(), symbols.end(),
                       [&pair_filter](const std::string& symbol) {
                         return pair_filter.find(symbol) == pair_filter.end();
                       });
    symbols.erase(end, symbols.end());
  };

  if (m_config.capture_book()) {
    const request::subscribe_book_t subscribe_book{
        ++m_book_req_id, m_config.book_depth(), true, symbols};
    m_session.send(subscribe_book.str());
  }

  if (m_config.capture_trades()) {
    const request::subscribe_trade_t subscribe_trade{++m_trade_req_id, true,
                                                     symbols};
    m_session.send(subscribe_trade.str());
  }

  return true;
}

bool engine_t::handle_instrument_update(
    const response::instrument_t& instrument) {
  m_sink.accept(instrument);
  return true;
}

bool engine_t::handle_book_msg(doc_t &doc) {
  auto buffer = std::string_view{};
  if (doc[c_header_type].get(buffer) != simdjson::SUCCESS) {
    BOOST_LOG_TRIVIAL(error)
        << __FUNCTION__ << ": missing 'type' " << simdjson::to_json_string(doc);
    return false;
  }

  if (buffer != c_book_type_snapshot && buffer != c_book_type_update) {
    BOOST_LOG_TRIVIAL(error) << __FUNCTION__ << ": unknown 'type' " << buffer;
    return false;
  }

  const auto response = response::book_t::from_json(doc);
  m_sink.accept(response);
  return true;
}

bool engine_t::handle_trade_msg(doc_t &doc) {
  const auto response = response::trades_t::from_json(doc);
  m_sink.accept(response);
  return true;
}

bool engine_t::handle_heartbeat_msg(doc_t &) {
  // !@# TODO: track a stat on time between heartbeats?
  return true;
}

bool engine_t::handle_pong_msg(doc_t & /*doc*/) {
  // !@# TODO: track ping/pong latency
  if (!m_subscribed) {
    const request::subscribe_instrument_t subscribe_inst{++m_inst_req_id};
    m_session.send(subscribe_inst.str());
    m_subscribed = true;
  }
  BOOST_LOG_TRIVIAL(info) << m_metrics.str();
  return true;
}

} // namespace krakpot
