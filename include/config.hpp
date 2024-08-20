#pragma once

#include "depth.hpp"
#include "types.hpp"

#include <boost/json.hpp>
#include <simdjson.h>

#include <cstdint>
#include <string>
#include <unordered_set>

namespace kdr {

struct config_t final {
  using symbol_filter_t = std::unordered_set<std::string>;

  static const std::string_view c_book_depth;
  static const std::string_view c_capture_book;
  static const std::string_view c_capture_trades;
  static const std::string_view c_kraken_host;
  static const std::string_view c_kraken_port;
  static const std::string_view c_pair_filter;
  static const std::string_view c_parquet_dir;
  static const std::string_view c_ping_interval_secs;

  static const std::string c_book_depth_str;
  static const std::string c_capture_book_str;
  static const std::string c_capture_trades_str;
  static const std::string c_kraken_host_str;
  static const std::string c_kraken_port_str;
  static const std::string c_pair_filter_str;
  static const std::string c_parquet_dir_str;
  static const std::string c_ping_interval_secs_str;

  config_t() {}

  config_t(size_t ping_interval_secs, std::string kraken_host,
           std::string kraken_port, symbol_filter_t pair_filter,
           std::string parquet_dir, model::depth_t book_depth,
           bool capture_book, bool capture_trades)
      : m_ping_interval_secs{ping_interval_secs},
        m_kraken_host{std::move(kraken_host)},
        m_kraken_port{std::move(kraken_port)},
        m_pair_filter{std::move(pair_filter)},
        m_parquet_dir{std::move(parquet_dir)}, m_book_depth{book_depth},
        m_capture_book{capture_book}, m_capture_trades{capture_trades} {}

  // !@# TODO: consider a c++20 concept for to_json/str behavior
  boost::json::object to_json_obj() const;
  std::string str() const { return boost::json::serialize(to_json_obj()); }

  static config_t from_json(simdjson::ondemand::document &doc);
  static config_t from_json_str(const std::string &json_str);

  size_t ping_interval_secs() const { return m_ping_interval_secs; }
  std::string kraken_host() const { return m_kraken_host; }
  std::string kraken_port() const { return m_kraken_port; }
  const symbol_filter_t &pair_filter() const { return m_pair_filter; }
  std::string parquet_dir() const { return m_parquet_dir; }
  model::depth_t book_depth() const { return m_book_depth; }
  bool capture_book() const { return m_capture_book; }
  bool capture_trades() const { return m_capture_trades; }

private:
  static constexpr size_t c_default_ping_interval_secs = 30;

  size_t m_ping_interval_secs = c_default_ping_interval_secs;
  std::string m_kraken_host = "ws.kraken.com";
  std::string m_kraken_port = "443";
  symbol_filter_t m_pair_filter;
  std::string m_parquet_dir = "/tmp";
  model::depth_t m_book_depth = model::depth_1000;
  bool m_capture_book = true;
  bool m_capture_trades = true;
};

} // namespace kdr
