/* Copyright (C) 2024 John C. Finley - All rights reserved */

#include "config.hpp"

#include <algorithm>

namespace krakpot {

config_t::config_t(size_t ping_interval_secs,
                   std::string kraken_host,
                   std::string kraken_port,
                   symbol_filter_t pair_filter,
                   std::string parquet_dir,
                   depth_t book_depth,
                   bool capture_book,
                   bool capture_trades)
    : m_ping_interval_secs{ping_interval_secs},
      m_kraken_host{kraken_host},
      m_kraken_port{kraken_port},
      m_pair_filter(pair_filter),
      m_parquet_dir{parquet_dir},
      m_book_depth{book_depth},
      m_capture_book{capture_book},
      m_capture_trades{capture_trades} {}

bool config_t::operator==(const config_t& rhs) const {
  return m_ping_interval_secs == rhs.m_ping_interval_secs &&
         m_kraken_host == rhs.m_kraken_host &&
         m_kraken_port == rhs.m_kraken_port &&
         m_pair_filter == rhs.m_pair_filter &&
         m_parquet_dir == rhs.m_parquet_dir &&
         m_book_depth == rhs.m_book_depth &&
         m_capture_book == rhs.m_capture_book &&
         m_capture_trades == rhs.m_capture_trades;
}

boost::json::object config_t::to_json_obj() const {
  auto pair_filter_array = boost::json::array{};
  std::transform(
      pair_filter().begin(), pair_filter().end(),
      std::back_inserter(pair_filter_array),
      [](const std::string& pair) { return boost::json::string{pair}; });

  const boost::json::object result = {
      {c_book_depth, book_depth()},
      {c_capture_book, capture_book()},
      {c_capture_trades, capture_trades()},
      {c_kraken_host, kraken_host()},
      {c_kraken_port, kraken_port()},
      {c_pair_filter, pair_filter_array},
      {c_parquet_dir, parquet_dir()},
      {c_ping_interval_secs, ping_interval_secs()}};
  return result;
}

config_t config_t::from_json(const boost::json::object& doc) {
  config_t result;

  if (doc.contains(c_ping_interval_secs)) {
    result.m_ping_interval_secs =
        doc.at(c_ping_interval_secs).to_number<size_t>();
  }
  if (doc.contains(c_kraken_host)) {
    result.m_kraken_host = doc.at(c_kraken_host).as_string();
  }
  if (doc.contains(c_kraken_port)) {
    result.m_kraken_port = doc.at(c_kraken_port).as_string();
  }
  if (doc.contains(c_pair_filter)) {
    for (const auto& symbol : doc.at(c_pair_filter).as_array()) {
      result.m_pair_filter.emplace(symbol.as_string());
    }
  }
  if (doc.contains(c_parquet_dir)) {
    result.m_parquet_dir = doc.at(c_parquet_dir).as_string();
  }
  if (doc.contains(c_book_depth)) {
    result.m_book_depth = depth_t{doc.at(c_book_depth).as_int64()};
  }

  return result;
}

config_t config_t::from_json_str(const std::string& json_str) {
  const boost::json::object doc = boost::json::parse(json_str).as_object();
  return from_json(doc);
}

}  // namespace krakpot
