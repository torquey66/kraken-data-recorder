/* Copyright (C) 2024 John C. Finley - All rights reserved */
#pragma once

#include <limits>

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

using price_t = double_t;
using qty_t = double_t;
using quote_t = std::pair<price_t, qty_t>;
using ask_t = quote_t;
using bid_t = quote_t;

static constexpr double_t c_NaN = std::numeric_limits<double>::signaling_NaN();

} // namespace krakpot
