#pragma once

#include "decimal.hpp"

#include <string>
#include <unordered_map>

namespace krakpot {

/**
 * Internal representation of the "Response Data-Pairs" field desribed here:
 *
 *     https://docs.kraken.com/websockets-v2/#instrument
 */
struct pair_t final {

  // I have not yet found a precise description of the integer type
  // referenced in the Kraken V2 API docs. A signed 64-bit int is
  // almost certainly sufficient for now.
  //
  // TODO: Research this and concoct an appropriate type.
  using integer_t = uint64_t;

  // clang-format off
  enum status_t : uint8_t {
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

  static pair_t from_json(std::string_view);

  std::string base() const { return m_base; }
  decimal_t cost_min() const { return m_cost_min; }
  integer_t cost_precision() const { return m_cost_precision; }
  bool has_index() const { return m_has_index; }
  decimal_t margin_initial() const { return m_margin_initial; }
  bool marginable() const { return m_marginable; }
  integer_t position_limit_long() const { return m_position_limit_long; }
  integer_t position_limit_short() const { return m_position_limit_short; }
  decimal_t price_increment() const { return m_price_increment; }
  integer_t price_precision() const { return m_price_precision; }
  decimal_t qty_increment() const { return m_qty_increment; }
  decimal_t qty_min() const { return m_qty_min; }
  integer_t qty_precision() const { return m_qty_precision; }
  std::string quote() const { return m_quote; }
  status_t status() const { return m_status; }
  std::string symbol() const { return m_symbol; }

private:
  static const std::unordered_map<std::string, status_t> c_str_to_status;
  static const std::unordered_map<status_t, std::string> c_status_to_str;

  std::string m_base;
  decimal_t m_cost_min;
  integer_t m_cost_precision = 0;
  bool m_has_index = false;
  decimal_t m_margin_initial;
  bool m_marginable = false;
  integer_t m_position_limit_long = 0;
  integer_t m_position_limit_short = 0;
  decimal_t m_price_increment;
  integer_t m_price_precision = 0;
  decimal_t m_qty_increment;
  decimal_t m_qty_min;
  integer_t m_qty_precision = 0;
  std::string m_quote;
  status_t m_status;
  std::string m_symbol;
};

} // namespace krakpot