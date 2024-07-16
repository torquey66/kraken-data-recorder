#pragma once

#include "types.hpp"

#include <boost/json.hpp>

namespace krakpot {

inline decimal_t extract_decimal(const boost::json::value& value) {
  if (value.is_string()) {
    return decimal_t{std::string_view{value.as_string()}};
  }
  if (value.is_double()) {
    const auto double_str = std::to_string(value.as_double());
    return decimal_t{double_str};
  }
  if (value.is_int64()) {
    const auto double_str = std::to_string(value.as_int64());
    return decimal_t{double_str};
  }
  if (value.is_uint64()) {
    const auto double_str = std::to_string(value.as_uint64());
    return decimal_t{double_str};
  }
  throw std::runtime_error("cannot convert value to decimal_t");
  /*
  boost::json::value result = value;
  result.emplace_string();
  return decimal_t{std::string_view{result.as_string()}};
  */
}

inline double_t extract_double(const boost::json::value& value) {
  /*
  if (value.is_double()) {
    return value.as_double();
  }
  if (value.is_int64()) {
    return double_t{value.as_int64()};
  }
  if (value.is_uint64()) {
    return double_t{value.as_uint64()};
  }
  throw std::runtime_error("cannot convert value to double_t");
  */
  boost::json::value result = value;
  result.emplace_double();
  return result.as_double();
}

}  // namespace krakpot
