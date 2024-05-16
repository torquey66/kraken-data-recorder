/* Copyright (C) 2024 John C. Finley - All rights reserved */

#include "responses.hpp"

#include <chrono>

namespace {

template <typename O>
krakpot::decimal_t extract_decimal(O &obj, std::string field) {
  const auto token = std::string_view{obj[field].raw_json_token()};
  const auto result = krakpot::decimal_t{obj[field].get_double(), token};
  return result;
}

} // namespace

namespace krakpot {
namespace response {

nlohmann::json header_t::to_json() const {
  const nlohmann::json result = {
      {c_header_recv_tm, recv_tm().str()}, {c_header_channel, channel()}, {c_header_type, type()}};
  return result;
}

instrument_t instrument_t::from_json(simdjson::ondemand::document &response) {
  auto result = instrument_t{};
  auto buffer = std::string_view{};

  buffer = response[c_header_channel].get_string();
  const auto channel = std::string{buffer.begin(), buffer.end()};
  buffer = response[c_header_type].get_string();
  const auto type = std::string{buffer.begin(), buffer.end()};
  result.m_header = header_t{timestamp_t::now(), channel, type};

  for (simdjson::fallback::ondemand::object obj : response[c_response_data][c_instrument_assets]) {
    const auto asset = asset_t::from_json(obj);
    result.m_assets.push_back(asset);
  }

  for (simdjson::fallback::ondemand::object obj : response[c_response_data][c_instrument_pairs]) {
    const auto pair = pair_t::from_json(obj);
    result.m_pairs.push_back(pair);
  }

  return result;
}

nlohmann::json instrument_t::to_json() const {

  auto assets = nlohmann::json{};
  for (const auto &asset : m_assets) {
    assets.push_back(asset.to_json());
  }

  auto pairs = nlohmann::json{};
  for (const auto &pair : m_pairs) {
    pairs.push_back(pair.to_json());
  }

  auto data = nlohmann::json{};
  data[c_instrument_pairs] = pairs;
  data[c_instrument_assets] = assets;

  const nlohmann::json result = {{c_header_channel, m_header.channel()},
                                 {c_response_data, data},
                                 {c_header_type, m_header.type()}};

  return result;
}

book_t::book_t(const header_t &header, const asks_t &asks, const bids_t &bids,
               uint64_t crc32, std::string symbol, timestamp_t timestamp)
    : m_header(header), m_asks(asks), m_bids(bids), m_crc32(crc32),
      m_symbol(symbol), m_timestamp(timestamp) {}

book_t book_t::from_json(simdjson::ondemand::document &response) {
  auto result = book_t{};
  auto buffer = std::string_view{};

  buffer = response[c_header_channel].get_string();
  const auto channel = std::string{buffer.begin(), buffer.end()};
  buffer = response[c_header_type].get_string();
  const auto type = std::string{buffer.begin(), buffer.end()};
  result.m_header = header_t{timestamp_t::now(), channel, type};

  // TODO: it is entirely unclear to me why `data` is an array since
  // it only ever seems to contain a single entry.
  bool processed = false;
  for (auto data : response[c_response_data]) {
    if (processed) {
      throw std::runtime_error(
          "TODO: fix problem with multiple data[] entries");
    }
    processed = true;

    // TODO: eliminate duplication
    for (simdjson::fallback::ondemand::object obj : data[c_book_asks]) {
      const auto price = extract_decimal(obj, c_book_price);
      const auto qty = extract_decimal(obj, c_book_qty);
      const ask_t ask = std::make_pair(price, qty);
      result.m_asks.push_back(ask);
    }

    for (simdjson::fallback::ondemand::object obj : data[c_book_bids]) {
      const auto price = extract_decimal(obj, c_book_price);
      const auto qty = extract_decimal(obj, c_book_qty);
      const bid_t bid = std::make_pair(price, qty);
      result.m_bids.push_back(bid);
    }

    result.m_crc32 = data[c_book_checksum].get_uint64();

    buffer = data[c_book_symbol].get_string();
    const auto symbol = std::string{buffer.begin(), buffer.end()};
    result.m_symbol = symbol;

    if (type == c_book_type_update) {
      buffer = data[c_book_timestamp].get_string();
      result.m_timestamp =
          timestamp_t::from_iso_8601(std::string{buffer.data(), buffer.size()});
    }
  }

  return result;
}

nlohmann::json book_t::to_json() const {

  auto asks = nlohmann::json::array();
  for (const auto &ask : m_asks) {
    const nlohmann::json ask_json = {{c_book_price, ask.first.value()},
                                     {c_book_qty, ask.second.value()}};
    asks.push_back(ask_json);
  }

  auto bids = nlohmann::json::array();
  for (const auto &bid : m_bids) {
    const nlohmann::json bid_json = {{c_book_price, bid.first.value()},
                                     {c_book_qty, bid.second.value()}};
    bids.push_back(bid_json);
  }

  auto content = nlohmann::json{};
  content[c_book_asks] = asks;
  content[c_book_bids] = bids;
  content[c_book_checksum] = m_crc32;
  content[c_book_symbol] = m_symbol;
  content[c_book_timestamp] = m_timestamp.str();

  auto data = nlohmann::json::array();
  data.push_back(content);

  const nlohmann::json result = {{c_header_channel, m_header.channel()},
                                 {c_response_data, data},
                                 {c_header_type, m_header.type()}};

  return result;
}

ord_type_t trades_t::parse_ord_type(std::string_view ord_type) {
  if (ord_type == c_trades_market) {
    return e_market;
  }
  if (ord_type == c_trades_limit) {
    return e_limit;
  }
  throw std::runtime_error{"unsupported ord_type: " +
                           std::string{ord_type.data(), ord_type.size()}};
}

side_t trades_t::parse_side(std::string_view side) {
  if (side == c_trades_buy) {
    return e_buy;
  }
  if (side == c_trades_sell) {
    return e_sell;
  }
  throw std::runtime_error{"unsupported side: " +
                           std::string{side.data(), side.size()}};
}

trades_t trades_t::from_json(simdjson::ondemand::document &response) {
  auto result = trades_t{};
  auto buffer = std::string_view{};

  buffer = response[c_header_channel].get_string();
  const auto channel = std::string{buffer.begin(), buffer.end()};
  buffer = response[c_header_type].get_string();
  const auto type = std::string{buffer.begin(), buffer.end()};
  result.m_header = header_t{timestamp_t::now(), channel, type};

  for (auto obj : response[c_response_data]) {
    auto trade = trade_t{};
    buffer = obj[c_trades_ord_type].get_string();
    trade.ord_type = parse_ord_type(buffer);
    trade.price = extract_decimal(obj, c_book_price);
    trade.qty = extract_decimal(obj, c_book_qty);
    buffer = obj[c_trades_side].get_string();
    trade.side = parse_side(buffer);
    buffer = obj[c_book_symbol].get_string();
    trade.symbol = std::string(buffer.begin(), buffer.end());
    buffer = obj[c_book_timestamp].get_string();
    trade.timestamp =
        timestamp_t::from_iso_8601(std::string{buffer.data(), buffer.size()});
    trade.trade_id = obj[c_trades_trade_id].get_uint64();
    result.m_trades.push_back(trade);
  }

  return result;
}

nlohmann::json trades_t::to_json() const {

  auto trades = nlohmann::json::array();
  for (const auto &trade : m_trades) {
    const nlohmann::json trade_json = {
        {c_trades_ord_type, trade.ord_type}, {c_book_price, trade.price.value()},
        {c_book_qty, trade.qty.value()},   {c_trades_side, trade.side},
        {c_book_symbol, trade.symbol},     {c_book_timestamp, trade.timestamp.str()},
        {c_trades_trade_id, trade.trade_id},
    };
    trades.push_back(trade_json);
  }

  const nlohmann::json result = {{c_header_channel, m_header.channel()},
                                 {c_response_data, trades},
                                 {c_header_type, m_header.type()}};
  return result;
}

} // namespace response
} // namespace krakpot
