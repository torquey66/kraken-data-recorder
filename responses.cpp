/* Copyright (C) 2024 John C. Finley - All rights reserved */

#include "responses.hpp"

#include <chrono>

namespace krakpot {
namespace response {

instrument_t instrument_t::from_json(simdjson::ondemand::document &response) {
  auto result = instrument_t{};
  auto buffer = std::string_view{};

  buffer = response["channel"].get_string();
  const auto channel = std::string{buffer.begin(), buffer.end()};
  buffer = response["type"].get_string();
  const auto type = std::string{buffer.begin(), buffer.end()};
  result.m_header = header_t{timestamp_t::now(), channel, type};

  for (simdjson::fallback::ondemand::object obj : response["data"]["assets"]) {
    const auto asset = asset_t::from_json(obj);
    result.m_assets.push_back(asset);
  }

  for (simdjson::fallback::ondemand::object obj : response["data"]["pairs"]) {
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
  data["pairs"] = pairs;
  data["assets"] = assets;

  const nlohmann::json result = {{"channel", m_header.channel()},
                                 {"data", data},
                                 {"type", m_header.type()}};

  return result;
}

book_t book_t::from_json(simdjson::ondemand::document &response) {
  auto result = book_t{};
  auto buffer = std::string_view{};

  buffer = response["channel"].get_string();
  const auto channel = std::string{buffer.begin(), buffer.end()};
  buffer = response["type"].get_string();
  const auto type = std::string{buffer.begin(), buffer.end()};
  result.m_header = header_t{timestamp_t::now(), channel, type};

  // TODO: it is entirely unclear to me why `data` is an array since
  // it only ever seems to contain a single entry.
  bool processed = false;
  for (auto data : response["data"]) {
    if (processed) {
      throw std::runtime_error(
          "TODO: fix problem with multiple data[] entries");
    }
    processed = true;

    // TODO: eliminate duplication
    for (simdjson::fallback::ondemand::object obj : data["asks"]) {
      const auto price_token = std::string_view{obj["price"].raw_json_token()};
      const auto price = price_t{obj["price"].get_double(), price_token};
      const auto qty_token = std::string_view{obj["qty"].raw_json_token()};
      const auto qty = qty_t{obj["qty"].get_double(), qty_token};
      const ask_t ask = std::make_pair(price, qty);
      result.m_asks.push_back(ask);
    }

    for (simdjson::fallback::ondemand::object obj : data["bids"]) {
      const auto price_token = std::string_view{obj["price"].raw_json_token()};
      const auto price = price_t{obj["price"].get_double(), price_token};
      const auto qty_token = std::string_view{obj["qty"].raw_json_token()};
      const auto qty = qty_t{obj["qty"].get_double(), qty_token};
      const bid_t bid = std::make_pair(price, qty);
      result.m_bids.push_back(bid);
    }

    result.m_crc32 = data["checksum"].get_uint64();

    buffer = data["symbol"].get_string();
    const auto symbol = std::string{buffer.begin(), buffer.end()};
    result.m_symbol = symbol;

    if (type == "update") {
      buffer = data["timestamp"].get_string();
      result.m_timestamp =
          timestamp_t::from_iso_8601(std::string{buffer.data(), buffer.size()});
    }
  }

  return result;
}

nlohmann::json book_t::to_json() const {

  auto asks = nlohmann::json::array();
  for (const auto &ask : m_asks) {
    const nlohmann::json ask_json = {{"price", ask.first.value()},
                                     {"qty", ask.second.value()}};
    asks.push_back(ask_json);
  }

  auto bids = nlohmann::json::array();
  for (const auto &bid : m_bids) {
    const nlohmann::json bid_json = {{"price", bid.first.value()},
                                     {"qty", bid.second.value()}};
    bids.push_back(bid_json);
  }

  auto content = nlohmann::json{};
  content["asks"] = asks;
  content["bids"] = bids;
  content["checksum"] = m_crc32;
  content["symbol"] = m_symbol;
  content["timestamp"] = m_timestamp.str();

  auto data = nlohmann::json::array();
  data.push_back(content);

  const nlohmann::json result = {{"channel", m_header.channel()},
                                 {"data", data},
                                 {"type", m_header.type()}};

  return result;
}

ord_type_t trades_t::parse_ord_type(std::string_view ord_type) {
  if (ord_type == "market") {
    return e_market;
  }
  if (ord_type == "limit") {
    return e_limit;
  }
  throw std::runtime_error{"unsupported ord_type: " +
                           std::string{ord_type.data(), ord_type.size()}};
}

side_t trades_t::parse_side(std::string_view side) {
  if (side == "buy") {
    return e_buy;
  }
  if (side == "sell") {
    return e_sell;
  }
  throw std::runtime_error{"unsupported side: " +
                           std::string{side.data(), side.size()}};
}

trades_t trades_t::from_json(simdjson::ondemand::document &response) {
  auto result = trades_t{};
  auto buffer = std::string_view{};

  buffer = response["channel"].get_string();
  const auto channel = std::string{buffer.begin(), buffer.end()};
  buffer = response["type"].get_string();
  const auto type = std::string{buffer.begin(), buffer.end()};
  result.m_header = header_t{timestamp_t::now(), channel, type};

  for (auto obj : response["data"]) {
    auto trade = trade_t{};
    buffer = obj["ord_type"].get_string();
    trade.ord_type = parse_ord_type(buffer);
    const auto price_token = std::string_view{obj["price"].raw_json_token()};
    trade.price = price_t{obj["price"].get_double(), price_token};
    const auto qty_token = std::string_view{obj["qty"].raw_json_token()};
    trade.qty = qty_t{obj["qty"].get_double(), qty_token};
    buffer = obj["side"].get_string();
    trade.side = parse_side(buffer);
    buffer = obj["symbol"].get_string();
    trade.symbol = std::string(buffer.begin(), buffer.end());
    buffer = obj["timestamp"].get_string();
    trade.timestamp =
        timestamp_t::from_iso_8601(std::string{buffer.data(), buffer.size()});
    trade.trade_id = obj["trade_id"].get_uint64();
    result.m_trades.push_back(trade);
  }

  return result;
}

nlohmann::json trades_t::to_json() const {

  auto trades = nlohmann::json::array();
  for (const auto &trade : m_trades) {
    const nlohmann::json trade_json = {
        {"ord_type", trade.ord_type}, {"price", trade.price.value()},
        {"qty", trade.qty.value()},   {"side", trade.side},
        {"symbol", trade.symbol},     {"timestamp", trade.timestamp.str()},
        {"trade_id", trade.trade_id},
    };
    trades.push_back(trade_json);
  }

  const nlohmann::json result = {{"channel", m_header.channel()},
                                 {"data", trades},
                                 {"type", m_header.type()}};
  return result;
}

} // namespace response
} // namespace krakpot
