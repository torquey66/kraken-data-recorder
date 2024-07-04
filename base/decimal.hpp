/* Copyright (C) 2024 John C. Finley - All rights reserved */
#pragma once

#include "constants.hpp"

#include <boost/crc.hpp>

#include <array>
#include <cmath>
#include <cstdio>
#include <string>
#include <string_view>

namespace krakpot {

/*
struct token_t final {
template <typename S>
token_t(S chars) {
  const auto chars_size = size_t(chars.end() - chars.begin());
  if (chars_size > m_chars.size()) {
    throw std::runtime_error("token overflow");
  }
  m_chars.fill('\0');
  std::copy(chars.begin(), chars.end(), m_chars.begin());
}

bool operator==(const token_t& rhs) const = default;

std::string str() const {
  return std::string{m_chars.data(), m_chars.size()};
}

void process(boost::crc_32_type&, int64_t) const;

private:
std::array<char, 24> m_chars;
};
*/

struct decimal_t final {
  // TODO: remove redundant definition with types.hpp
  using integer_t = int64_t;

  decimal_t() : m_value{c_NaN} {}
  explicit decimal_t(double value) : m_value{value} {}

  auto operator<=>(const decimal_t& rhs) const {
    return m_value <=> rhs.m_value;
  }
  bool operator==(const decimal_t& rhs) const = default;

  double value() const { return m_value; }

  integer_t scaled_value(integer_t precision) const {
    double result = m_value;
    while (precision-- > 0) {
      result *= 10;
    }
    return static_cast<integer_t>(std::round(result));
  }

  auto str(integer_t precision) const {
    std::array<char, 64> buffer;
    const std::string fmt{"%." + std::to_string(precision) + "f"};
    const auto num_written =
        std::snprintf(buffer.data(), buffer.size(), fmt.c_str(), value());
    return std::string(buffer.data(), num_written);
  }

  void process(boost::crc_32_type& crc32, integer_t precision) const {
    const std::string buffer{std::to_string(scaled_value(precision))};
    for (const auto ch : buffer) {
      crc32.process_byte(ch);
    }
  }

 private:
  double m_value = 0.0;
};

}  // namespace krakpot
