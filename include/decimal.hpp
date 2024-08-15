#pragma once

#include "constants.hpp"

#include <boost/crc.hpp>
#include <boost/multiprecision/cpp_dec_float.hpp>

#include <string>

namespace kdr {

// TODO: remove redundant definition with types.hpp
using integer_t = int64_t;

// Note: The value 72 is chosen such that sizeof(decimal_t) is 64 on
// my machine, which is not coincidentally its L1 cache size.
//
// TODO: figure out if there's a portable way to tune or at least
// verify this at compile time.
static constexpr size_t c_num_wide_float_digits = 72;

using wide_float_t = boost::multiprecision::number<
    boost::multiprecision::cpp_dec_float<c_num_wide_float_digits>>;

struct decimal_t final {
  decimal_t() : m_value{c_NaN} {}

  template <typename S>
  decimal_t(S str) : m_value{str} {}

  bool operator<(const decimal_t& rhs) const { return m_value < rhs.m_value; }
  bool operator>(const decimal_t& rhs) const { return m_value > rhs.m_value; }
  bool operator==(const decimal_t& rhs) const { return m_value == rhs.m_value; }
  bool operator!=(const decimal_t& rhs) const { return m_value != rhs.m_value; }

  wide_float_t value() const { return m_value; }

  double double_value(integer_t precision) const {
    return std::strtod(str(precision).c_str(), nullptr);
  }

  std::string str(integer_t precision) const;

  void process(boost::crc_32_type& crc32, integer_t precision) const;

 private:
  wide_float_t m_value = 0.0;
};

}  // namespace kdr
