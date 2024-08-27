#include "engine.hpp"

#include "book.hpp"
#include "constants.hpp"
#include "header.hpp"
#include "instrument.hpp"
#include "pong.hpp"
#include "requests.hpp"

#include <boost/log/trivial.hpp>
#include <simdjson.h>

#include <algorithm>
#include <cassert>
#include <vector>

namespace kdr {

engine_t::engine_t(ssl_context_t &ssl_context, const config_t &config,
                   const sink_t &sink)
    : m_session{ssl_context, config}, m_config{config},
      m_metrics_timer{m_session.ioc()}, m_process_timer{m_session.ioc()},
      m_sink(sink) {}

void engine_t::start_processing(const recv_cb_t &recv_cb) {

  const auto connected_cb = [this]() {
    BOOST_LOG_TRIVIAL(debug) << "engine_t connected!";

    BOOST_LOG_TRIVIAL(debug) << "starting metrics timer...";
    m_metrics_timer.expires_from_now(
        boost::posix_time::seconds(c_metrics_interval_secs));
    m_metrics_timer.async_wait(
        [this](error_code ec) { this->on_metrics_timer(ec); });

    BOOST_LOG_TRIVIAL(debug) << "starting processing timer...";
    m_process_timer.expires_after(
        std::chrono::microseconds(c_process_interval_micros));
    m_process_timer.async_wait(
        [this](error_code ec) { this->on_process_timer(ec); });
  };

  m_session.start_processing(connected_cb, recv_cb);
}

bool engine_t::handle_msg(msg_t msg) {
  m_metrics.accept(msg);

  try {
    simdjson::padded_string padded_msg{msg};
    simdjson::ondemand::document doc = m_parser.iterate(padded_msg);

    auto buffer = std::string_view{};
    if (doc[c_response_channel].get(buffer) == simdjson::SUCCESS) {
      if (buffer == c_channel_instrument) {
        return handle_instrument_msg(doc);
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

  } catch (const std::exception &ex) {
    BOOST_LOG_TRIVIAL(error)
        << __FUNCTION__ << ": " << ex.what() << " msg: " << msg;
    return false;
  }

  return true;
}

bool engine_t::handle_instrument_msg(doc_t &doc) {
  auto buffer = std::string_view{};
  if (doc[response::header_t::c_type].get(buffer) != simdjson::SUCCESS) {
    BOOST_LOG_TRIVIAL(error)
        << __FUNCTION__ << ": missing 'type' " << simdjson::to_json_string(doc);
    return false;
  }

  if (buffer == c_instrument_snapshot) {
    return handle_instrument_snapshot(doc);
  } else if (buffer == c_instrument_update) {
    return handle_instrument_update(doc);
  }

  BOOST_LOG_TRIVIAL(error) << __FUNCTION__ << ": unknown 'type' " << buffer;
  return false;
}

bool engine_t::handle_instrument_snapshot(doc_t &doc) {
  const auto response = response::instrument_t::from_json(doc);
  m_sink.accept(response);

  const auto &pairs = response.pairs();
  auto symbols = std::vector<std::string>{};
  std::transform(pairs.begin(), pairs.end(), std::back_inserter(symbols),
                 [](const auto &pair) { return pair.symbol(); });

  const auto &pair_filter = m_config.pair_filter();
  if (!pair_filter.empty()) {
    const auto end =
        std::remove_if(symbols.begin(), symbols.end(),
                       [&pair_filter](const std::string &symbol) {
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

bool engine_t::handle_instrument_update(doc_t &doc) {
  const auto response = response::instrument_t::from_json(doc);
  m_sink.accept(response);
  return true;
}

bool engine_t::handle_book_msg(doc_t &doc) {
  m_book_responses.push(response::book_t::from_json(doc));
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

bool engine_t::handle_pong_msg(doc_t &doc) {
  // !@# TODO: Move this to a dedicated one-shot timer
  if (!m_subscribed) {
    const request::subscribe_instrument_t subscribe_inst{++m_inst_req_id};
    m_session.send(subscribe_inst.str());
    m_subscribed = true;
  }

  simdjson::fallback::ondemand::object obj = doc.get_object();
  const auto pong = model::pong_t::from_json(obj);
  BOOST_LOG_TRIVIAL(info) << pong.str();

  // Can't reliably track ping/pong latency since pongs do not include a reqid.
  m_metrics.pong();
  return true;
}

void engine_t::on_metrics_timer(error_code ec) {
  if (ec) {
    BOOST_LOG_TRIVIAL(error) << __FUNCTION__ << " " << ec.message();
  }
  BOOST_LOG_TRIVIAL(info) << m_metrics.str();
  m_metrics_timer.expires_from_now(
      boost::posix_time::seconds(c_metrics_interval_secs));
  m_metrics_timer.async_wait(
      [this](error_code ec) { this->on_metrics_timer(ec); });
}

void engine_t::on_process_timer(error_code ec) {
  if (ec) {
    BOOST_LOG_TRIVIAL(error) << __FUNCTION__ << " " << ec.message();
  }

  if (!m_book_responses.empty()) {
    m_sink.accept(m_book_responses.front());
    m_book_responses.pop();
  }

  m_metrics.set_book_queue_depth(m_book_responses.size());

  m_process_timer.expires_after(
      std::chrono::microseconds(c_process_interval_micros));
  m_process_timer.async_wait(
      [this](error_code ec) { this->on_process_timer(ec); });
}

} // namespace kdr
