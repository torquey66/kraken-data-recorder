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

  static constexpr std::string_view c_book_depth = "book_depth";
  static constexpr std::string_view c_capture_book = "capture_book";
  static constexpr std::string_view c_capture_trades = "capture_trades";
  static constexpr std::string_view c_kraken_host = "kraken_host";
  static constexpr std::string_view c_kraken_port = "kraken_port";
  static constexpr std::string_view c_pair_filter = "pair_filter";
  static constexpr std::string_view c_parquet_dir = "parquet_dir";
  static constexpr std::string_view c_ping_interval_secs = "ping_interval_secs";
  static constexpr std::string_view c_enable_shmem = "enable_shmem";

  config_t() {}

  config_t(size_t ping_interval_secs, std::string kraken_host,
           std::string kraken_port, symbol_filter_t pair_filter,
           std::string parquet_dir, model::depth_t book_depth,
           bool capture_book, bool capture_trades, bool enable_shmem)
      : m_ping_interval_secs{ping_interval_secs},
        m_kraken_host{std::move(kraken_host)},
        m_kraken_port{std::move(kraken_port)},
        m_pair_filter{std::move(pair_filter)},
        m_parquet_dir{std::move(parquet_dir)}, m_book_depth{book_depth},
        m_capture_book{capture_book}, m_capture_trades{capture_trades},
        m_enable_shmem{enable_shmem} {}

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
  bool enable_shmem() const { return m_enable_shmem; }

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
  bool m_enable_shmem = false;
};

} // namespace kdr
