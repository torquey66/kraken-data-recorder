#include "decimal.hpp"

#include <algorithm>

namespace krakpot {

void token_t::process(boost::crc_32_type &crc32) const {
  auto in_leading_zeros = true;
  for (const auto ch : m_chars) {
    if (std::isdigit(ch)) {
      if (!in_leading_zeros) {
        crc32.process_byte(ch);
      } else if (ch != '0') {
        in_leading_zeros = false;
        crc32.process_byte(ch);
      }
    }
  }
}

} // namespace krakpot
