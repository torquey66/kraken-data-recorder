#include "decimal.hpp"

#include <sstream>

namespace kdr {

std::string format_frac_part(wide_float_t frac_part, integer_t precision) {
  std::string result;
  for (auto counter = 0; counter < precision; ++counter) {
    static constexpr uint64_t frac_part_multiplier = 10;
    frac_part *= frac_part_multiplier;
    const auto int_part = frac_part.convert_to<uint64_t>();
    result += std::to_string(int_part);
    frac_part -= int_part;
  }
  return result;
}

std::string decimal_t::str() const {
  std::ostringstream os;
  os << m_value;
  return os.str();
}

std::string decimal_t::str(integer_t precision) const {
  if (m_value.is_zero()) {
    if (precision > 0) {
      std::string str = "0.";
      str.append(precision, '0');
      return str;
    }
    return "0";
  }

  if (m_value.sign() == -1) {
    const wide_float_t abs_value{-m_value};
    return "-" + abs_value.str(precision);
  }

  const wide_float_t int_part = boost::multiprecision::floor(m_value);
  const wide_float_t frac_part = m_value - int_part;
  const auto int_str = std::to_string(int_part.convert_to<uint64_t>());
  const auto frac_str = format_frac_part(frac_part, precision);
  return int_str + (frac_str.empty() ? "" : "." + frac_str);
}

void decimal_t::process(boost::crc_32_type &crc32, int64_t precision) const {
  const auto chars = str(precision);
  auto in_leading_zeros = true;
  auto in_trailing_digits = false;
  auto num_trailing_digits = int64_t{0};
  for (const auto ch : chars) {
    if (in_trailing_digits && num_trailing_digits >= precision) {
      return;
    }
    if (std::isdigit(ch) != 0) {
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
}

} // namespace kdr
