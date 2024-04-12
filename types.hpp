/* Copyright (C) 2024 John C. Finley - All rights reserved */
#pragma once

#include <cstdint>
#include <limits>
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

using integer_t = int64_t;
using double_t = double;

using req_id_t = int64_t;
using timestamp_t = uint64_t; // microseconds since the epoch

using price_t = double_t;
using qty_t = double_t;
using quote_t = std::pair<price_t, qty_t>;
using ask_t = quote_t;
using bid_t = quote_t;

static constexpr double_t c_NaN = std::numeric_limits<double>::signaling_NaN();

/**
 * ord_type_t is based on the FIX 5.0 spec. See:
 *
 * https://www.onixs.biz/fix-dictionary/5.0/tagNum_40.html
 */
enum ord_type_t : char {
  // clang-format off
  e_market                = '1',
  e_limit                 = '2',
  e_stop                  = '3',
  e_stop_limit            = '4',
  e_with_or_without       = '6',
  e_limit_with_or_without = '8',
  e_on_basis              = '9',
  e_previously_quoted     = 'D',
  e_previously_indicated  = 'E',
  // clang-format off
};

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

} // namespace krakpot
