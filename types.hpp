#pragma once

#include <limits>

namespace krakpot {

// I have not yet found a precise description of the integer type
// referenced in the Kraken V2 API docs. A signed 64-bit int is
// almost certainly sufficient for now.
//
// TODO: Research this and concoct an appropriate type.
using integer_t = uint64_t;
using double_t = double;

static constexpr double_t c_NaN = std::numeric_limits<double>::signaling_NaN();

} // namespace krakpot
