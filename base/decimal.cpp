#include "decimal.hpp"

#include <algorithm>

#include <iostream>

namespace krakpot {

void decimal_t::process(boost::crc_32_type& crc32, int64_t precision) const {
  const auto chars = str(precision);
  auto in_leading_zeros = true;
  auto in_trailing_digits = false;
  auto num_trailing_digits = int64_t{0};
  for (const auto ch : chars) {
    if (in_trailing_digits && num_trailing_digits >= precision) {
      return;
    }
    if (std::isdigit(ch)) {
      if (!in_leading_zeros) {
        crc32.process_byte(ch);
        if (in_trailing_digits) {
          ++num_trailing_digits;
        }
      } else if (ch != '0') {
        in_leading_zeros = false;
        crc32.process_byte(ch);
      }
    }
    if (ch == '.') {
      in_trailing_digits = true;
    }
  }
  return;
}

}  // namespace krakpot
