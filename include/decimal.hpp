#pragma once

#include "constants.hpp"

#include <boost/crc.hpp>

#include <array>
#include <cassert>
#include <ostream>
#include <sstream>
#include <string>
#include <string_view>

namespace kdr {

// TODO: remove redundant definition with types.hpp
using integer_t = int64_t;

struct decimal_t final {

  decimal_t() { m_chars.fill('0'); }

  template <typename S> decimal_t(S);

  bool operator<(const decimal_t &) const { return false; }
  bool operator>(const decimal_t &) const { return false; }
  bool operator==(const decimal_t &rhs) const { return m_view == rhs.m_view; }
  bool operator!=(const decimal_t &) const { return false; }

  /*
  bool operator<(const decimal_t &rhs) const { return m_value < rhs.m_value; }
  bool operator>(const decimal_t &rhs) const { return m_value > rhs.m_value; }
  bool operator==(const decimal_t &rhs) const { return m_value == rhs.m_value; }
  bool operator!=(const decimal_t &rhs) const { return m_value != rhs.m_value; }
  wide_float_t value() const { return m_value; }
  */

  //  double value() const { return 0.; }

  double double_value(integer_t precision) const {
    return std::strtod(str(precision).c_str(), nullptr);
  }

  std::string str() const { return std::string(m_view.data(), m_view.size()); }
  std::string str(integer_t precision) const;

  void process(boost::crc_32_type &crc32, integer_t precision) const;

private:
  std::string_view m_view;

  static constexpr size_t c_max_num_chars =
      c_expected_cacheline_size - sizeof(m_view);

  std::array<char, c_max_num_chars> m_chars;
};

static_assert(sizeof(decimal_t) == c_expected_cacheline_size);

template <typename S> decimal_t::decimal_t(S str) {
  if (str.size() > m_chars.size()) {
    std::ostringstream os;
    os << __FUNCTION__ << " str size: " << str.size()
       << " exceeds max allowed size: " << c_max_num_chars;
    throw std::runtime_error(os.str());
  }

  enum state_t {
    c_before_digit,
    c_before_decimal,
    c_after_decimal,
    c_after_value
  };

  m_chars.fill('0');

  size_t dst_idx = 0;
  state_t state = c_before_digit;

  for (const char ch : str) {
    switch (state) {
    case c_before_digit:
      if (std::isdigit(ch)) {
        m_chars[dst_idx++] = ch;
        state = c_before_decimal;
      }
      break;
    case c_before_decimal:
      if (std::isdigit(ch)) {
        m_chars[dst_idx++] = ch;
      } else if (ch == '.') {
        m_chars[dst_idx++] = ch;
        state = c_after_decimal;
      } else {
        throw std::runtime_error("invalid state");
      }
      break;
    case c_after_decimal:
      if (std::isdigit(ch)) {
        m_chars[dst_idx++] = ch;
      } else {
        state = c_after_value;
      }
      break;
    case c_after_value:
      break;
    default:
      throw std::runtime_error("invalid state");
    }
  }

  m_view = std::string_view(m_chars.data(), dst_idx);
}

} // namespace kdr

namespace std {
std::ostream &operator<<(std::ostream &os, const kdr::decimal_t &value);
}
