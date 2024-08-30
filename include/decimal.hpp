#pragma once

#include "constants.hpp"

#include <boost/crc.hpp>

#include <algorithm>
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

  decimal_t() : m_view(c_zero_chars.data(), c_zero_chars.size()) {}

  decimal_t(const decimal_t &rhs) {
    std::copy(rhs.m_chars.begin(), rhs.m_chars.end(), m_chars.begin());
    m_view = std::string_view(m_chars.data(), rhs.m_view.size());
  }

  decimal_t(const decimal_t &&rhs)
      : m_chars(std::move(rhs.m_chars)),
        m_view(m_chars.data(), rhs.m_view.size()) {}

  decimal_t &operator=(const decimal_t &rhs) {
    std::copy(rhs.m_chars.begin(), rhs.m_chars.end(), m_chars.begin());
    m_view = std::string_view(m_chars.data(), rhs.m_view.size());
    return *this;
  }

  decimal_t &operator=(const decimal_t &&) = delete;

  template <typename S> decimal_t(S);

  auto operator<=>(const decimal_t &rhs) const;

  bool operator<(const decimal_t &rhs) const;
  bool operator>(const decimal_t &rhs) const;
  bool operator==(const decimal_t &rhs) const;
  bool operator!=(const decimal_t &rhs) const;

  double double_value(integer_t precision) const {
    return std::strtod(str(precision).c_str(), nullptr);
  }

  std::string str() const { return std::string(m_view.data(), m_view.size()); }
  std::string str(integer_t precision) const;
  std::string_view str_view(integer_t precision) const;

  void process(boost::crc_32_type &crc32, integer_t precision) const;

private:
  std::string_view int_part() const;
  std::string_view frac_part() const;

  bool is_zero() const;

  static constexpr size_t c_max_num_chars =
      c_expected_cacheline_size - sizeof(std::string_view);

  mutable std::array<char, c_max_num_chars> m_chars;
  std::string_view m_view;

  static constexpr std::array<char, 3> c_zero_chars = {'0', '.', '0'};
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

  for (size_t idx = 0; idx < str.size(); ++idx) {
    const char ch = str[idx];
    switch (state) {
    case c_before_digit:
      if (std::isdigit(ch)) {
        if (ch == '0') {
          if ((str.size() - idx) > 1) {
            if (str[idx + 1] == '.') {
              m_chars[dst_idx++] = ch;
              state = c_before_decimal;
            }
          } else {
            m_chars[dst_idx++] = ch;
            state = c_after_value;
          }
        } else {
          m_chars[dst_idx++] = ch;
          state = c_before_decimal;
        }
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
  if (is_zero()) {
    m_view = std::string_view(c_zero_chars.data(), c_zero_chars.size());
    return;
  }

  const auto idx = m_view.find('.');
  if (idx != std::string_view::npos) {
    auto end_idx = m_view.size() - 1;
    while (m_view[end_idx] == '0' && end_idx >= idx) {
      --end_idx;
    }
    if (m_view[end_idx] == '.') {
      --end_idx;
    }
    m_view = m_view.substr(0, end_idx + 1);
  }
}

inline std::string_view decimal_t::int_part() const {
  const auto decimal_point = m_view.find('.');
  return m_view.substr(0, decimal_point);
}

inline std::string_view decimal_t::frac_part() const {
  const auto decimal_point = m_view.find('.');
  if (decimal_point == std::string_view::npos) {
    return std::string_view();
  }
  return m_view.substr(decimal_point + 1);
}

} // namespace kdr

namespace std {
std::ostream &operator<<(std::ostream &os, const kdr::decimal_t &value);
}
