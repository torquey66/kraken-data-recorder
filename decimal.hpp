/* Copyright (C) 2024 John C. Finley - All rights reserved */
#pragma once

#include <algorithm>
#include <array>
#include <sstream>

namespace krakpot {

struct decimal_t final {
  template <typename S>
  explicit decimal_t(double value, S token)
      : m_value{value}, m_token{token.begin(), token.end()} {}

  auto operator<=>(const decimal_t &rhs) const {
    return m_value <=> rhs.m_value;
  }
  bool operator==(const decimal_t &rhs) const = default;

  auto value() const { return m_value; }
  auto token() const { return m_token; }

  auto str() const { return m_token; }

private:
  double m_value;
  std::string m_token;
};

} // namespace krakpot
