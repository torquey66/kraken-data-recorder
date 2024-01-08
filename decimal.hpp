#pragma once

#include <algorithm>
#include <array>
#include <cassert>
#include <string>

namespace krakpot {

struct decimal_t final {

  static constexpr size_t max_len_c = 16;

  template <typename S> decimal_t(S value) {
    assert(value.size() <= m_value.size());
    m_value.fill('\0');
    std::copy_n(m_value.data(), m_value.size(), m_value.begin());
  }

private:
  std::array<char, max_len_c> m_value;
};

} // namespace krakpot
