/* Copyright (C) 2024 John C. Finley - All rights reserved */
#pragma once

#include "asset.hpp"
#include "pair.hpp"

#include <nlohmann/json.hpp>
#include <simdjson.h>

#include <string>
#include <vector>

/**
 * These are the various messages we currently receive from the
 * venue. Ideally, this code would be generated from an OpenAPI spec
 * or the like, but that exercise is currently beyond scope. Also,
 * according to ChatGPT, Kraken has not released an OpenAPI spec for
 * its websocket interface.
 */
namespace krakpot {
namespace response {

/**
 * Fields common to all channel messages.
 */
struct header_t final {
  header_t() = default;
  header_t(timestamp_t recv_tm, std::string channel, std::string type)
      : m_recv_tm(recv_tm), m_channel(channel), m_type(type) {}

  const timestamp_t recv_tm() const { return m_recv_tm; }
  const std::string &channel() const { return m_channel; }
  const std::string &type() const { return m_type; }

private:
  timestamp_t m_recv_tm;
  std::string m_channel;
  std::string m_type;
};

/**
 * See https://docs.kraken.com/websockets-v2/#instrument
 */
struct instrument_t final {
  instrument_t() = default;

  static instrument_t from_json(simdjson::ondemand::document &);

  nlohmann::json to_json() const;
  std::string str() const { return to_json().dump(); }

  const std::vector<asset_t> &assets() const { return m_assets; }
  const std::vector<pair_t> &pairs() const { return m_pairs; }

private:
  header_t m_header;
  std::vector<asset_t> m_assets;
  std::vector<pair_t> m_pairs;
};

/**
 * https://docs.kraken.com/websockets-v2/#book
 */
struct book_t final {
  book_t() = default;

  static book_t from_json(simdjson::ondemand::document &);

  nlohmann::json to_json() const;
  std::string str() const { return to_json().dump(); }

private:
  using asks_t = std::vector<ask_t>;
  using bids_t = std::vector<bid_t>;

  header_t m_header;
  asks_t m_asks;
  bids_t m_bids;
  uint64_t m_crc32;
  std::string m_symbol;
  timestamp_t m_timestamp = 0;
};

/**
 * https://docs.kraken.com/websockets-v2/#trade
 */
struct trades_t final {
  struct trade_t final {
    ord_type_t ord_type;
    price_t price;
    qty_t qty;
    side_t side;
    std::string symbol;
    timestamp_t timestamp;
    integer_t trade_id;
  };

  trades_t() = default;

  const header_t &header() const { return m_header; }

  static trades_t from_json(simdjson::ondemand::document &);

  nlohmann::json to_json() const;
  std::string str() const { return to_json().dump(); }

  auto begin() const { return m_trades.begin(); }
  auto end() const { return m_trades.end(); }
  auto size() const { return m_trades.size(); }

private:
  static ord_type_t parse_ord_type(std::string_view);
  static side_t parse_side(std::string_view);

  header_t m_header;
  std::vector<trade_t> m_trades;
};

} // namespace response
} // namespace krakpot
