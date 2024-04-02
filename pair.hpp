#pragma once

#include "types.hpp"

#include "nlohmann/json.hpp"
#include <simdjson.h>

#include <optional>
#include <string>
#include <unordered_map>

namespace krakpot {
namespace response {

/**
 * Internal representation of the "Response Data-Pairs" field desribed here:
 *
 *     https://docs.kraken.com/websockets-v2/#instrument
 */
struct pair_t final {

  // clang-format off
  enum status_t : int8_t {
    e_invalid          = -1,
    e_cancel_only      = 0,
    e_delisted         = 1,
    e_limit_only       = 2,
    e_maintenance      = 3,
    e_online           = 4,
    e_post_only        = 5,
    e_reduce_only      = 6,
    e_work_in_progress = 7,
  };
  // clang-format on

  static pair_t from_json(simdjson::ondemand::object &);

  nlohmann::json to_json() const;
  std::string str() const { return to_json().dump(); }

  std::string base() const { return m_base; }
  double_t cost_min() const { return m_cost_min; }
  integer_t cost_precision() const { return m_cost_precision; }
  bool has_index() const { return m_has_index; }
  std::optional<double_t> margin_initial() const { return m_margin_initial; }
  bool marginable() const { return m_marginable; }
  std::optional<integer_t> position_limit_long() const {
    return m_position_limit_long;
  }
  std::optional<integer_t> position_limit_short() const {
    return m_position_limit_short;
  }
  double_t price_increment() const { return m_price_increment; }
  integer_t price_precision() const { return m_price_precision; }
  double_t qty_increment() const { return m_qty_increment; }
  double_t qty_min() const { return m_qty_min; }
  integer_t qty_precision() const { return m_qty_precision; }
  std::string quote() const { return m_quote; }
  status_t status() const { return m_status; }
  std::string symbol() const { return m_symbol; }

private:
  static const std::unordered_map<std::string, status_t> c_str_to_status;
  static const std::unordered_map<status_t, std::string> c_status_to_str;

  std::string m_base;
  double_t m_cost_min = c_NaN;
  integer_t m_cost_precision = 0;
  bool m_has_index = false;
  std::optional<double_t> m_margin_initial;
  bool m_marginable = false;
  std::optional<integer_t> m_position_limit_long;
  std::optional<integer_t> m_position_limit_short;
  double_t m_price_increment = c_NaN;
  integer_t m_price_precision = 0;
  double_t m_qty_increment = c_NaN;
  double_t m_qty_min = c_NaN;
  integer_t m_qty_precision = 0;
  std::string m_quote;
  status_t m_status = e_invalid;
  std::string m_symbol;
};

} // namespace response
} // namespace krakpot
