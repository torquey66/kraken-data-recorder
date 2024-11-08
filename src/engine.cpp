#include "engine.hpp"

#include "book.hpp"
#include "constants.hpp"
#include "header.hpp"
#include "instrument.hpp"
#include "pong.hpp"

#include <boost/log/trivial.hpp>
#include <simdjson.h>

#include <algorithm>
#include <cassert>
#include <ranges>
#include <vector>

namespace kdr {

engine_t::engine_t(ssl_context_t &ssl_context, const config_t &config,
                   const sink_t &sink)
    : m_session{ssl_context, config}, m_config{config},
      m_metrics_timer{m_session.ioc()}, m_ping_timer{m_session.ioc()},
      m_process_timer{m_session.ioc()}, m_sink(sink) {

  if (m_config.ping_interval_secs() < 1) {
    BOOST_LOG_TRIVIAL(error)
        << __FUNCTION__
        << " ping_interval_secs (=" << m_config.ping_interval_secs()
        << ") must be at least one";
  }
}

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
    m_process_timer.async_wait([this](error_code ec) {
      this->on_process_timer(ec);
      // Defer subscriptions until we know we're processing
      const request::subscribe_instrument_t subscribe_inst{++m_inst_req_id};
      m_session.send(subscribe_inst.str());
    });

    BOOST_LOG_TRIVIAL(debug) << "starting ping timer...";
    m_ping_timer.expires_from_now(
        boost::posix_time::seconds(m_config.ping_interval_secs()));
    m_ping_timer.async_wait([this](error_code ec) { this->on_ping_timer(ec); });
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
      assert(buffer == c_method_pong || buffer = c_method_subscribe);
      if (buffer == c_method_pong) {
        // We have to crack the message to know that it's a pong, but
        // then we have to reparse it so that the pong_t deserializer
        // can see the method field.
        simdjson::padded_string padded_pong_msg{msg};
        simdjson::ondemand::document pong_doc =
            m_parser.iterate(padded_pong_msg);
        return handle_pong_msg(pong_doc);
      }
      if (buffer == c_method_subscribe) {
        // !@# TODO: ultimately we will want to crack open 'subscribe'
        simdjson::padded_string padded_msg{msg};
        simdjson::ondemand::document doc = m_parser.iterate(padded_msg);
        BOOST_LOG_TRIVIAL(debug)
            << __FUNCTION__ << ": " << simdjson::to_json_string(doc);
        return true;
      }
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

  auto begin = symbols.begin();
  while (begin != symbols.end()) {
    // !@# TODO: clean up a little...
    auto end =
        begin + std::min(size_t{32}, static_cast<std::size_t>(
                                         std::distance(begin, symbols.end())));
    if (m_config.capture_book()) {
      const request::subscribe_book_t subscribe_book{
          ++m_book_req_id, m_config.book_depth(), true,
          std::vector<std::string>{begin, end}};
      m_book_subs.push(subscribe_book);
    }

    if (m_config.capture_trades()) {
      const request::subscribe_trade_t subscribe_trade{
          ++m_trade_req_id, true, std::vector<std::string>{begin, end}};
      m_trade_subs.push(subscribe_trade);
    }

    begin = end;
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
  m_metrics.heartbeat();
  return true;
}

bool engine_t::handle_pong_msg(doc_t &doc) {
  simdjson::fallback::ondemand::object obj = doc.get_object();
  const auto pong = model::pong_t::from_json(obj);
  BOOST_LOG_TRIVIAL(info) << pong.str();

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

void engine_t::on_ping_timer(error_code ec) {
  if (ec) {
    BOOST_LOG_TRIVIAL(error) << __FUNCTION__ << " " << ec.message();
  }

  const auto ping = request::ping_t{++m_ping_req_id};
  const auto msg = ping.str();
  m_session.send(msg);
  m_metrics.ping();

  m_ping_timer.expires_from_now(
      boost::posix_time::seconds(m_config.ping_interval_secs()));
  m_ping_timer.async_wait([this](error_code ec) { this->on_ping_timer(ec); });
}

void engine_t::on_process_timer(error_code ec) {
  if (ec) {
    BOOST_LOG_TRIVIAL(error) << __FUNCTION__ << " " << ec.message();
  }

  m_metrics.set_book_queue_depth(m_book_responses.size());
  size_t num_to_process =
      std::min(c_process_batch_size, m_book_responses.size());
  if (num_to_process > 0) {
    size_t num_processed = 0;
    const timestamp_t begin = timestamp_t::now();
    while (num_to_process > 0) {
      m_sink.accept(m_book_responses.front());
      m_book_responses.pop();
      --num_to_process;
      ++num_processed;
    }
    const timestamp_t end = timestamp_t::now();
    m_metrics.set_book_last_process_micros(end.micros() - begin.micros());
    m_metrics.set_book_last_consumed(num_processed);
  }

  if (!m_book_subs.empty()) {
    const auto &book_sub = m_book_subs.front();
    m_session.send(book_sub.str());
    m_book_subs.pop();
  }

  if (!m_trade_subs.empty()) {
    const auto &trade_sub = m_trade_subs.front();
    m_session.send(trade_sub.str());
    m_trade_subs.pop();
  }

  m_process_timer.expires_after(
      std::chrono::microseconds(c_process_interval_micros));
  m_process_timer.async_wait(
      [this](error_code ec) { this->on_process_timer(ec); });
}

} // namespace kdr
