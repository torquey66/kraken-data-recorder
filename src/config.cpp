#include "config.hpp"

#include <algorithm>
#include <array>

namespace {

std::string to_string(std::string_view sv) {
  return std::string{sv.data(), sv.size()};
}

}  // namespace

namespace kdr {

static constexpr auto c_book_depth_field = std::to_array("book_depth");
static constexpr auto c_capture_book_field = std::to_array("capture_book");
static constexpr auto c_capture_trades_field = std::to_array("capture_trades");
static constexpr auto c_kraken_host_field = std::to_array("kraken_host");
static constexpr auto c_kraken_port_field = std::to_array("kraken_port");
static constexpr auto c_pair_filter_field = std::to_array("pair_filter");
static constexpr auto c_parquet_dir_field = std::to_array("parquet_dir");
static constexpr auto c_ping_interval_secs_field =
    std::to_array("ping_interval_secs");

/*******************************************************************************/

const std::string_view config_t::c_book_depth{c_book_depth_field.data(),
                                              c_book_depth_field.size() - 1};
const std::string_view config_t::c_capture_book{
    c_capture_book_field.data(), c_capture_book_field.size() - 1};
const std::string_view config_t::c_capture_trades{
    c_capture_trades_field.data(), c_capture_trades_field.size() - 1};
const std::string_view config_t::c_kraken_host{c_kraken_host_field.data(),
                                               c_kraken_host_field.size() - 1};
const std::string_view config_t::c_kraken_port{c_kraken_port_field.data(),
                                               c_kraken_port_field.size() - 1};
const std::string_view config_t::c_pair_filter{c_pair_filter_field.data(),
                                               c_pair_filter_field.size() - 1};
const std::string_view config_t::c_parquet_dir{c_parquet_dir_field.data(),
                                               c_parquet_dir_field.size() - 1};
const std::string_view config_t::c_ping_interval_secs{
    c_ping_interval_secs_field.data(), c_ping_interval_secs_field.size() - 1};

/*******************************************************************************/

const std::string config_t::c_book_depth_str{c_book_depth_field.data(),
                                             c_book_depth_field.size() - 1};
const std::string config_t::c_capture_book_str{c_capture_book_field.data(),
                                               c_capture_book_field.size() - 1};
const std::string config_t::c_capture_trades_str{
    c_capture_trades_field.data(), c_capture_trades_field.size() - 1};
const std::string config_t::c_kraken_host_str{c_kraken_host_field.data(),
                                              c_kraken_host_field.size() - 1};
const std::string config_t::c_kraken_port_str{c_kraken_port_field.data(),
                                              c_kraken_port_field.size() - 1};
const std::string config_t::c_pair_filter_str{c_pair_filter_field.data(),
                                              c_pair_filter_field.size() - 1};
const std::string config_t::c_parquet_dir_str{c_parquet_dir_field.data(),
                                              c_parquet_dir_field.size() - 1};
const std::string config_t::c_ping_interval_secs_str{
    c_ping_interval_secs_field.data(), c_ping_interval_secs_field.size() - 1};

/*******************************************************************************/

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
      {c_ping_interval_secs, ping_interval_secs()},
  };
  return result;
}

config_t config_t::from_json(simdjson::ondemand::document& doc) {
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
    const int64_t val = optional_val.get_int64();
    result.m_book_depth = model::depth_t{val};
  }

  return result;
}

config_t config_t::from_json_str(const std::string& json_str) {
  simdjson::ondemand::parser parser;
  simdjson::padded_string padded{json_str};
  simdjson::ondemand::document doc = parser.iterate(padded);
  return from_json(doc);
}

}  // namespace kdr
