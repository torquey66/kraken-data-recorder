/* Copyright (C) 2024 John C. Finley - All rights reserved */
#pragma once

#include "decimal.hpp"

#include <cstdint>
#include <limits>
#include <string>
#include <utility>

/**
 * The Kraken API spec references types such as 'float' and 'integer'
 * for which I have not yet found precise definitions. For now, I've
 * chosen common types with the widest ranges for internal
 * representations.
 *
 * TODO: I believe these types might derive implicitly from OpenAPI,
 * but I have not found a definitive answer to that. More research is
 * in order.
 */
namespace krakpot {

using msg_t = std::string_view;

using integer_t = int64_t;
using double_t = double;

using req_id_t = int64_t;
using price_t = decimal_t;
using qty_t = decimal_t;
using quote_t = std::pair<price_t, qty_t>;
using ask_t = quote_t;
using bid_t = quote_t;

/**
 * side_t is based on the FIX 5.0 spec. See:
 *
 * https://www.onixs.biz/fix-dictionary/5.0/tagNum_54.html
 */
enum side_t : char {
  // clang-format off
  e_buy                = '1',
  e_sell               = '2',
  e_buy_minus          = '3',
  e_sell_plus          = '4',
  e_sell_short         = '5',
  e_sell_short_exempt  = '6',
  e_undisclosed        = '7',
  e_cross              = '8',
  e_cross_short        = '9',
  e_cross_short_exempt = 'A',
  e_as_defined         = 'B',
  e_opposite           = 'C',
  e_subscribe          = 'D',
  e_redeem             = 'E',
  e_lend               = 'F',
  e_borrow             = 'G',
  // clang-format on
};

/**
 * AFAIK, Kraken timetamps are always ISO 8601 strings in GMT.
 *
 * TODO: consider using chrono timepoints in some fashion instead of
 * raw micros.
 */
struct timestamp_t final {
  timestamp_t() {}
  timestamp_t(int64_t in_micros) : m_micros(in_micros) {}

  std::string str() const { return to_iso_8601(m_micros); };

  int64_t micros() const { return m_micros; }

  static std::string to_iso_8601(int64_t);
  static int64_t from_iso_8601(const std::string &);

  static timestamp_t now();

private:
  int64_t m_micros = 0;
};

}  // namespace krakpot
