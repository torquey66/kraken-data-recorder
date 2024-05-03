/* Copyright (C) 2024 John C. Finley - All rights reserved */
#pragma once

#include "constants.hpp"

#include <string>
#include <string_view>

namespace krakpot {

struct token_t final {

  template <typename S>
  token_t(S chars) : m_chars{chars.begin(), chars.end()} {}

  token_t(const char *str) : m_chars{str} {}

  bool operator==(const token_t &rhs) const = default;

  std::string trimmed() const;

  std::string str() const { return m_chars; }

private:
  std::string m_chars;
};

struct decimal_t final {

  decimal_t() : m_value{c_NaN}, m_token{c_NaN_str} {}

  template <typename S>
  explicit decimal_t(double value, S token) : m_value{value}, m_token{token} {}

  auto operator<=>(const decimal_t &rhs) const {
    return m_value <=> rhs.m_value;
  }
  bool operator==(const decimal_t &rhs) const = default;

  auto value() const { return m_value; }
  auto token() const { return m_token; }

  auto str() const { return m_token.str(); }

private:
  double m_value;
  token_t m_token;
};

} // namespace krakpot
