#include "decimal.hpp"

namespace krakpot {

std::string format_frac_part(wide_float_t frac_part, precision_t precision) {
  std::string result;
  for (auto counter = 0; counter < precision; ++counter) {
    frac_part *= 10;
  }
  frac_part = boost::multiprecision::round(frac_part);
  result = std::to_string(frac_part.convert_to<uint64_t>());
  if (size_t{precision} > result.size()) {
    const size_t num_leading_zeroes = size_t{precision} - result.size();
    result.insert(0, num_leading_zeroes, '0');
  }
  return result;
}

std::string decimal_t::str() const {
  if (m_value.is_zero()) {
    if (m_precision > 0) {
      std::string str = "0.";
      str.append(m_precision, '0');
      return str;
    } else {
      return "0";
    }
  } else if (m_value.sign() == -1) {
    const wide_float_t abs_value{-m_value};
    return "-" + abs_value.str(m_precision);
  }

  const wide_float_t int_part = boost::multiprecision::floor(m_value);
  const wide_float_t frac_part = m_value - int_part;
  const auto int_str = std::to_string(int_part.convert_to<uint64_t>());
  const auto frac_str = format_frac_part(frac_part, m_precision);
  return int_str + (frac_str.empty() ? "" : "." + frac_str);
}

void decimal_t::process(boost::crc_32_type& crc32) const {
  const auto chars = str();
  auto in_leading_zeros = true;
  auto in_trailing_digits = false;
  auto num_trailing_digits = int64_t{0};
  for (const auto ch : chars) {
    if (in_trailing_digits && num_trailing_digits >= m_precision) {
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
