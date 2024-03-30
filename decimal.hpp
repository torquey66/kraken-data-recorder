#pragma once

#include <algorithm>
#include <array>
#include <cassert>
#include <cstdlib>
#include <limits>
#include <string>

namespace krakpot {

struct decimal_t final {

  static constexpr size_t max_len_c = 24;

  decimal_t() { m_value.fill('\0'); }

  decimal_t(const decimal_t &) = default;

  // TODO: define a concept for this
  template <typename S> decimal_t(S value) {
    assert(value.size() <= m_value.size());
    m_value.fill('\0');
    std::copy_n(m_value.data(), m_value.size(), m_value.begin());
    m_double = std::strtod(m_value.data(), nullptr);
    assert(errno != ERANGE);
  }

  double as_double() const { return m_double; }

  std::string str() const {
    return std::string{m_value.begin(), m_value.end()};
  }

  decimal_t &operator=(const decimal_t &rhs) = default;

  bool operator<(const decimal_t &rhs) const { return m_double < rhs.m_double; }
  bool operator>(const decimal_t &rhs) const { return m_double > rhs.m_double; }
  bool operator==(const decimal_t &rhs) const {
    return m_double == rhs.m_double;
  }

private:
  std::array<char, max_len_c> m_value;
  double m_double = std::numeric_limits<double>::signaling_NaN();
};

} // namespace krakpot
