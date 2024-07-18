/* Copyright (C) 2024 John C. Finley - All rights reserved */
#pragma once

#include "constants.hpp"

#include <boost/crc.hpp>
#include <boost/multiprecision/cpp_dec_float.hpp>

#include <ostream>
#include <string>

#include <iostream>

namespace krakpot {

using wide_float_t =
    boost::multiprecision::number<boost::multiprecision::cpp_dec_float<64>>;

using precision_t = uint8_t;

struct decimal_t final {
  // Note: The value 72 is chosen such that sizeof(decimal_t) is 64 on
  // my machine, which is not coincidentally its L1 cache size.
  //
  // TODO: figure out if there's a portable way to tune or at least
  // verify this at compile time.
  decimal_t() = default;

  explicit decimal_t(int64_t value, precision_t precision)
      : m_value{value}, m_precision{precision} {}
  explicit decimal_t(uint64_t value, precision_t precision)
      : m_value{value}, m_precision{precision} {}
  explicit decimal_t(double value, precision_t precision)
      : m_value{value}, m_precision{precision} {}
  explicit decimal_t(wide_float_t value, precision_t precision)
      : m_value{value}, m_precision{precision} {}

  decimal_t(const char* str, precision_t precision)
      : m_value{std::string{str}}, m_precision{precision} {}
  decimal_t(const std::string str, precision_t precision)
      : m_value{str}, m_precision{precision} {}
  decimal_t(const std::string_view str, precision_t precision)
      : m_value{str}, m_precision{precision} {}

  // !@# TODO - see if we can eliminate the string conversion junk
  bool operator==(const decimal_t& rhs) const { return str() == rhs.str(); }
  bool operator!=(const decimal_t& rhs) const { return str() != rhs.str(); }

  bool operator<(const decimal_t& rhs) const { return m_value < rhs.m_value; }
  bool operator>(const decimal_t& rhs) const { return m_value > rhs.m_value; }

  wide_float_t value() const { return m_value; }
  precision_t precision() const { return m_precision; }

  double double_value() const { return std::strtod(str().c_str(), nullptr); }

  std::string str() const;

  void process(boost::crc_32_type& crc32) const;

 private:
  wide_float_t m_value = 0.0;
  precision_t m_precision = 0;
};

inline std::ostream& operator<<(std::ostream& os, const decimal_t& dt) {
  os << dt.value() << " " << dt.double_value() << " " << dt.str();
  return os;
}

static_assert(sizeof(decimal_t) == 64);

}  // namespace krakpot
