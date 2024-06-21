/* Copyright (C) 2024 John C. Finley - All rights reserved */
#pragma once

#include "types.hpp"

#include <simdjson.h>

#include <cstdint>
#include <string>

namespace krakpot {

struct config_t final {

  static constexpr char c_book_depth[] = "book_depth";
  static constexpr char c_capture_book[] = "capture_book";
  static constexpr char c_capture_trades[] = "capture_trades";
  static constexpr char c_kraken_host[] = "kraken_host";
  static constexpr char c_kraken_port[] = "kraken_port";
  static constexpr char c_parquet_dir[] = "parquet_dir";
  static constexpr char c_ping_interval_secs[] = "ping_interval_secs";

  config_t() {}

  config_t(size_t ping_interval_secs, std::string kraken_host,
           std::string kraken_port, std::string parquet_dir, depth_t book_depth,
           bool capture_book, bool capture_trades)
      : m_ping_interval_secs{ping_interval_secs}, m_kraken_host{kraken_host},
        m_kraken_port{kraken_port}, m_parquet_dir{parquet_dir},
        m_book_depth{book_depth}, m_capture_book{capture_book},
        m_capture_trades{capture_trades} {}

  static config_t from_json(simdjson::ondemand::document &);
  static config_t from_json_str(const std::string &);

  size_t ping_interval_secs() const { return m_ping_interval_secs; }
  std::string kraken_host() const { return m_kraken_host; }
  std::string kraken_port() const { return m_kraken_port; }
  std::string parquet_dir() const { return m_parquet_dir; }
  depth_t book_depth() const { return m_book_depth; }
  bool capture_book() const { return m_capture_book; }
  bool capture_trades() const { return m_capture_trades; }

private:
  size_t m_ping_interval_secs = 30;
  std::string m_kraken_host = "ws.kraken.com";
  std::string m_kraken_port = "443";
  std::string m_parquet_dir = "/tmp";
  depth_t m_book_depth = e_1000;
  bool m_capture_book = true;
  bool m_capture_trades = true;
};

} // namespace krakpot
