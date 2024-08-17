#pragma once

#include "decimal.hpp"
#include "timestamp.hpp"  // !@# TODO: remove this and include timestamp only where it is required

#include <cstdint>
#include <string_view>

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
namespace kdr {

using msg_t = std::string_view;

using integer_t = int64_t;
using double_t = double;

using req_id_t = int64_t;
using price_t = decimal_t;
using qty_t = decimal_t;
using quote_t = std::pair<price_t, qty_t>;
using ask_t = quote_t;
using bid_t = quote_t;

}  // namespace kdr
