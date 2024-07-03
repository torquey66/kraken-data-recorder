/* Copyright (C) 2024 John C. Finley - All rights reserved */
#pragma once

#include "constants.hpp"

#include <boost/crc.hpp>

#include <array>
#include <string>
#include <string_view>

namespace krakpot {

#pragma pack(push, 1)
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

struct decimal_t final {
  decimal_t() : m_value{c_NaN}, m_token{std::string{c_NaN_str}} {}

  template <typename S>
  explicit decimal_t(double value, S token) : m_value{value}, m_token{token} {}

  auto operator<=>(const decimal_t& rhs) const {
    return m_value <=> rhs.m_value;
  }
  bool operator==(const decimal_t& rhs) const = default;

  auto value() const { return m_value; }
  auto& token() const { return m_token; }

  auto str() const { return m_token.str(); }

  void process(boost::crc_32_type& crc32, int64_t precision) const {
    return m_token.process(crc32, precision);
  }

 private:
  double m_value;
  token_t m_token;
};
#pragma pack(pop)

}  // namespace krakpot
