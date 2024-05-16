/* Copyright (C) 2024 John C. Finley - All rights reserved */

#include "config.hpp"

namespace {

std::string to_string(std::string_view sv) {
  return std::string{sv.data(), sv.size()};
}

} // namespace

namespace krakpot {

config_t config_t::from_json(simdjson::ondemand::document &doc) {
  config_t result;

  simdjson::ondemand::value optional_val;

  if (doc[c_ping_interval_secs].get(optional_val) == simdjson::SUCCESS) {
    result.m_ping_interval_secs = optional_val.get_uint64();
  }

  if (doc[c_kraken_host].get(optional_val) == simdjson::SUCCESS) {
    result.m_kraken_host = ::to_string(optional_val.get_string());
  }

  if (doc[c_kraken_port].get(optional_val) == simdjson::SUCCESS) {
    result.m_kraken_port = ::to_string(optional_val.get_string());
  }

  if (doc[c_parquet_dir].get(optional_val) == simdjson::SUCCESS) {
    result.m_parquet_dir = ::to_string(optional_val.get_string());
  }

  if (doc[c_book_depth].get(optional_val) == simdjson::SUCCESS) {
    result.m_book_depth = optional_val.get_int64();
  }

  return result;
}

config_t config_t::from_json_str(const std::string &json_str) {
  simdjson::ondemand::parser parser;
  simdjson::padded_string padded{json_str};
  simdjson::ondemand::document doc = parser.iterate(padded);
  return from_json(doc);
}

} // namespace krakpot
