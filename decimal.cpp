#include "decimal.hpp"

#include <algorithm>

namespace krakpot {

std::string token_t::trimmed() const {
  // !@# TODO upgrade compiler and use ranges
  // !@# TODO verify that this is not a bottleneck
  auto buffer = m_chars;
  auto eit = std::remove(buffer.begin(), buffer.end(), '.');
  buffer.erase(eit, buffer.end());
  eit = std::remove_if(buffer.begin(), buffer.end(),
                       [](const char ch) { return std::iswspace(ch); });
  buffer.erase(eit, buffer.end());
  const auto begin = buffer.find_first_not_of('0');
  buffer = buffer.substr(begin);
  return buffer;
}

} // namespace krakpot
