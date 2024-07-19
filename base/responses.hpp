/* Copyright (C) 2024 John C. Finley - All rights reserved */
#pragma once

#include "asset.hpp"
#include "pair.hpp"

#include <boost/json.hpp>

#include <ostream>
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
  header_t(timestamp_t recv_tm, std::string channel, std::string type);

  bool operator==(const header_t&) const = default;

  const timestamp_t recv_tm() const { return m_recv_tm; }
  const std::string& channel() const { return m_channel; }
  const std::string& type() const { return m_type; }

  boost::json::object to_json_obj() const;
  std::string str() const { return boost::json::serialize(to_json_obj()); }

 private:
  timestamp_t m_recv_tm;
  std::string m_channel;
  std::string m_type;
};

inline std::ostream& operator<<(std::ostream& os, const header_t& header) {
  os << header.str();
  return os;
}

/**
 * See https://docs.kraken.com/websockets-v2/#instrument
 */
struct instrument_t final {
  instrument_t() = default;

  instrument_t(const header_t&,
               const std::vector<asset_t>&,
               const std::vector<pair_t>&);

  bool operator==(const instrument_t&) const = default;

  const header_t& header() const { return m_header; }
  const std::vector<asset_t>& assets() const { return m_assets; }
  const std::vector<pair_t>& pairs() const { return m_pairs; }

  boost::json::object to_json_obj() const;
  std::string str() const { return boost::json::serialize(to_json_obj()); }

  static instrument_t from_json_obj(const boost::json::object&);

 private:
  header_t m_header;
  std::vector<asset_t> m_assets;
  std::vector<pair_t> m_pairs;
};

/**
 * https://docs.kraken.com/websockets-v2/#book
 */
struct book_t final {
  using asks_t = std::vector<ask_t>;
  using bids_t = std::vector<bid_t>;

  book_t() = default;
  book_t(const header_t&,
         const asks_t&,
         const bids_t&,
         uint64_t,
         std::string,
         timestamp_t);

  const header_t& header() const { return m_header; }
  const bids_t& bids() const { return m_bids; }
  const asks_t& asks() const { return m_asks; }
  uint64_t crc32() const { return m_crc32; }
  const std::string& symbol() const { return m_symbol; }
  timestamp_t timestamp() const { return m_timestamp; }

  static book_t from_json_obj(const boost::json::object&, const pair_t&);

  boost::json::object to_json_obj() const;
  std::string str() const { return boost::json::serialize(to_json_obj()); }

 private:
  header_t m_header;
  asks_t m_asks;
  bids_t m_bids;
  uint64_t m_crc32;
  std::string m_symbol;
  timestamp_t m_timestamp;
};

inline std::ostream& operator<<(std::ostream& os, const book_t& book) {
  os << book.str();
  return os;
}

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

  const header_t& header() const { return m_header; }

  static trades_t from_json_obj(const boost::json::object&, const pair_t&);

  boost::json::object to_json_obj() const;
  std::string str() const { return boost::json::serialize(to_json_obj()); }

  auto begin() const { return m_trades.begin(); }
  auto end() const { return m_trades.end(); }
  auto size() const { return m_trades.size(); }

 private:
  static ord_type_t parse_ord_type(std::string_view);
  static side_t parse_side(std::string_view);

  header_t m_header;
  std::vector<trade_t> m_trades;
};

}  // namespace response
}  // namespace krakpot
