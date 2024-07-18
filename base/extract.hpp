#pragma once

#include "types.hpp"

#include <boost/json.hpp>

namespace krakpot {

inline decimal_t extract_decimal(const boost::json::value& value,
                                 precision_t precision) {
  if (value.is_string()) {
    return decimal_t{std::string_view{value.as_string()}, precision};
  }
  if (value.is_double()) {
    const auto double_str = std::to_string(value.as_double());
    return decimal_t{double_str, precision};
  }
  if (value.is_int64()) {
    const auto double_str = std::to_string(value.as_int64());
    return decimal_t{double_str, precision};
  }
  if (value.is_uint64()) {
    const auto double_str = std::to_string(value.as_uint64());
    return decimal_t{double_str, precision};
  }
  throw std::runtime_error("cannot convert value to decimal_t");
}

inline double_t extract_double(const boost::json::value& value) {
  if (value.is_double()) {
    return value.as_double();
  }
  // if (value.is_int64()) {
  //   return double_t{value.as_int64()};
  // }
  // if (value.is_uint64()) {
  //   return double_t{value.as_uint64()};
  // }
  throw std::runtime_error("cannot convert value to double_t");
}

inline precision_t extract_precision(const boost::json::value& value) {
  if (value.is_int64()) {
    return static_cast<precision_t>(value.as_int64());
  }
  if (value.is_uint64()) {
    return static_cast<precision_t>(value.as_uint64());
  }
  throw std::runtime_error("cannot convert value to precision_t");
}

}  // namespace krakpot
