#include "decimal.hpp"

#include <cstdlib>
#include <sstream>

namespace {
uint64_t to_int(std::string_view view) {
  uint64_t result = 0;
  for (const char ch : view) {
    result *= 10;
    result += ch - '0';
  }
  return result;
}

} // namespace

namespace kdr {

auto decimal_t::operator<=>(const decimal_t &rhs) const {
  const std::string_view lhs_int_view = int_part();
  const std::string_view rhs_int_view = rhs.int_part();
  const auto lhs_int = to_int(lhs_int_view);
  const auto rhs_int = to_int(rhs_int_view);
  auto result = lhs_int <=> rhs_int;
  if (result != std::strong_ordering::equal) {
    return result;
  }

  const std::string_view lhs_frac_view = frac_part();
  const std::string_view rhs_frac_view = rhs.frac_part();
  const auto size = std::max(lhs_frac_view.size(), rhs_frac_view.size());
  for (size_t idx = 0; idx < size; ++idx) {
    const auto lhs_digit =
        idx < lhs_frac_view.size() ? (lhs_frac_view[idx] - '0') : 0;
    const auto rhs_digit =
        idx < rhs_frac_view.size() ? (rhs_frac_view[idx] - '0') : 0;
    result = lhs_digit <=> rhs_digit;
    if (result != std::strong_ordering::equal) {
      return result;
    }
  }
  return std::strong_ordering::equal;
}

bool decimal_t::operator<(const decimal_t &rhs) const {
  return (*this <=> rhs) == std::strong_ordering::less;
}
bool decimal_t::operator>(const decimal_t &rhs) const {
  return (*this <=> rhs) == std::strong_ordering::greater;
}
bool decimal_t::operator==(const decimal_t &rhs) const {
  return (*this <=> rhs) == std::strong_ordering::equal;
}
bool decimal_t::operator!=(const decimal_t &rhs) const {
  return (*this <=> rhs) != std::strong_ordering::equal;
}

bool decimal_t::is_zero() const {
  const auto int_part_view = int_part();
  if (int_part_view.size() > 0 && to_int(int_part_view) != 0) {
    return false;
  }
  const auto frac_part_view = frac_part();
  if (frac_part_view.size() > 0 && to_int(frac_part_view) != 0) {
    return false;
  }
  return true;
}

std::string decimal_t::str(integer_t precision) const {
  if (precision < 0) {
    std::ostringstream os;
    os << "invalid precision: " << precision;
    throw std::runtime_error(os.str());
  }

  auto result = std::string(m_view.data(), m_view.size());
  const auto decimal_pos = result.find('.');

  if (decimal_pos == std::string::npos) {
    result += '.';
    result += std::string(precision, '0');
    return result;
  }

  const auto chars_after_decimal = result.size() - (decimal_pos + 1);
  if (size_t(precision) > chars_after_decimal) {
    const auto zeros_to_add = precision - chars_after_decimal;
    result += std::string(zeros_to_add, '0');
  } else {
    result = result.substr(0, decimal_pos + 1 + precision);
  }

  return result;
}

void decimal_t::process(boost::crc_32_type &crc32, int64_t precision) const {
  auto in_leading_zeros = true;
  auto in_trailing_digits = false;
  auto num_trailing_digits = int64_t{0};
  for (const auto ch : m_chars) {
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

namespace std {

std::ostream &operator<<(std::ostream &os, const kdr::decimal_t &value) {
  os << value.str();
  return os;
}
} // namespace std
