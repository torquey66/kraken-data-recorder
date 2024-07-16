/* Copyright (C) 2024 John C. Finley - All rights reserved */

#include "responses.hpp"

#include <algorithm>
#include <chrono>

namespace {

template <typename O>
krakpot::decimal_t extract_decimal(O& obj, std::string field) {
  const auto token = std::string_view{obj[field].raw_json_token()};
  const auto result = krakpot::decimal_t{token};
  return result;
}

}  // namespace

namespace krakpot {
namespace response {

header_t::header_t(timestamp_t recv_tm, std::string channel, std::string type)
    : m_recv_tm{recv_tm}, m_channel{channel}, m_type{type} {}

instrument_t::instrument_t(const header_t& header,
                           const std::vector<asset_t>& assets,
                           const std::vector<pair_t>& pairs)
    : m_header{header}, m_assets{assets}, m_pairs{pairs} {}

boost::json::object header_t::to_json_obj() const {
  const boost::json::object result = {{c_header_recv_tm, recv_tm().str()},
                                      {c_header_channel, channel()},
                                      {c_header_type, type()}};
  return result;
}

instrument_t instrument_t::from_json_obj(
    const boost::json::object& instrument_obj) {
  auto result = instrument_t{};
  const auto channel =
      std::string{instrument_obj.at(c_header_channel).as_string()};
  const auto type = std::string{instrument_obj.at(c_header_type).as_string()};
  // !@# TODO: consider deserializing req_tm if present for roundtrip tests,
  // etc.
  result.m_header = header_t{timestamp_t::now(), channel, type};

  const auto& data_obj = instrument_obj.at(c_response_data).as_object();

  for (const auto& asset_obj : data_obj.at(c_instrument_assets).as_array()) {
    const auto asset = asset_t::from_json_obj(asset_obj.as_object());
    result.m_assets.push_back(asset);
  }

  for (const auto& pair_obj : data_obj.at(c_instrument_pairs).as_array()) {
    const auto pair = pair_t::from_json_obj(pair_obj.as_object());
    result.m_pairs.push_back(pair);
  }

  return result;
}

boost::json::object instrument_t::to_json_obj() const {
  auto assets = boost::json::array{};
  std::transform(m_assets.begin(), m_assets.end(), std::back_inserter(assets),
                 [](const asset_t& asset) { return asset.to_json_obj(); });

  auto pairs = boost::json::array{};
  std::transform(m_pairs.begin(), m_pairs.end(), std::back_inserter(pairs),
                 [](const pair_t& pair) { return pair.to_json_obj(); });

  auto data = boost::json::object{};
  data[c_instrument_pairs] = pairs;
  data[c_instrument_assets] = assets;

  const boost::json::object result = {{c_header_channel, m_header.channel()},
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

boost::json::object book_t::to_json_obj(integer_t price_precision,
                                        integer_t qty_precision) const {
  auto asks = boost::json::array();
  std::transform(m_asks.begin(), m_asks.end(), std::back_inserter(asks),
                 [price_precision, qty_precision](const ask_t& ask) {
                   const boost::json::object result = {
                       {c_book_price, ask.first.double_value(price_precision)},
                       {c_book_qty, ask.second.double_value(qty_precision)}};
                   return result;
                 });

  auto bids = boost::json::array();
  std::transform(m_bids.begin(), m_bids.end(), std::back_inserter(bids),
                 [price_precision, qty_precision](const bid_t& bid) {
                   const boost::json::object result = {
                       {c_book_price, bid.first.double_value(price_precision)},
                       {c_book_qty, bid.second.double_value(qty_precision)}};
                   return result;
                 });

  auto content = boost::json::object{};
  content[c_book_asks] = asks;
  content[c_book_bids] = bids;
  content[c_book_checksum] = m_crc32;
  content[c_book_symbol] = m_symbol;
  content[c_book_timestamp] = m_timestamp.str();

  auto data = boost::json::array();
  data.push_back(content);

  const boost::json::object result = {{c_header_channel, m_header.channel()},
                                      {c_response_data, data},
                                      {c_header_type, m_header.type()}};

  return result;
}

ord_type_t trades_t::parse_ord_type(std::string_view ord_type) {
  if (ord_type == c_trade_market) {
    return e_market;
  }
  if (ord_type == c_trade_limit) {
    return e_limit;
  }
  throw std::runtime_error{"unsupported ord_type: " +
                           std::string{ord_type.data(), ord_type.size()}};
}

side_t trades_t::parse_side(std::string_view side) {
  if (side == c_trade_buy) {
    return e_buy;
  }
  if (side == c_trade_sell) {
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
    buffer = obj[c_trade_ord_type].get_string();
    trade.ord_type = parse_ord_type(buffer);
    trade.price = extract_decimal(obj, c_trade_price);
    trade.qty = extract_decimal(obj, c_trade_qty);
    buffer = obj[c_trade_side].get_string();
    trade.side = parse_side(buffer);
    buffer = obj[c_trade_symbol].get_string();
    trade.symbol = std::string(buffer.begin(), buffer.end());
    buffer = obj[c_trade_timestamp].get_string();
    trade.timestamp =
        timestamp_t::from_iso_8601(std::string{buffer.data(), buffer.size()});
    trade.trade_id = obj[c_trade_trade_id].get_uint64();
    result.m_trades.push_back(trade);
  }

  return result;
}

boost::json::object trades_t::to_json_obj(integer_t price_precision,
                                          integer_t qty_precision) const {
  auto trades = boost::json::array();
  std::transform(
      m_trades.begin(), m_trades.end(), std::back_inserter(trades),
      [price_precision, qty_precision](const trade_t& trade) {
        const boost::json::object result = {
            {c_trade_ord_type, trade.ord_type},
            {c_trade_price, trade.price.double_value(price_precision)},
            {c_trade_qty, trade.qty.double_value(qty_precision)},
            {c_trade_side, trade.side},
            {c_trade_symbol, trade.symbol},
            {c_trade_timestamp, trade.timestamp.str()},
            {c_trade_trade_id, trade.trade_id},
        };
        return result;
      });

  const boost::json::object result = {{c_header_channel, m_header.channel()},
                                      {c_response_data, trades},
                                      {c_header_type, m_header.type()}};
  return result;
}

} // namespace response
} // namespace krakpot
