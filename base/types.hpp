/* Copyright (C) 2024 John C. Finley - All rights reserved */
#pragma once

#include "decimal.hpp"

#include <cstdint>
#include <limits>
#include <string>
#include <utility>

/**
 * The Kraken API spec references types such as 'float' and 'integer'
 * for which I have not yet found precise definitions. For now, I've
 * chosen common types with the widest ranges for internal
 * representations.
 *
 * TODO: I believe these types might derive implicitly from OpenAPI,
 * but I have not found a definitive answer to that. More research is
 * in order.
 */
namespace krakpot {

using msg_t = std::string_view;

using integer_t = int64_t;
using double_t = double;

using req_id_t = int64_t;
using price_t = decimal_t;
using qty_t = decimal_t;
using quote_t = std::pair<price_t, qty_t>;
using ask_t = quote_t;
using bid_t = quote_t;

/**
 * AFAIK, Kraken timetamps are always ISO 8601 strings in GMT.
 *
 * TODO: consider using chrono timepoints in some fashion instead of
 * raw micros.
 */
struct timestamp_t final {
  timestamp_t() {}
  timestamp_t(int64_t in_micros) : m_micros(in_micros) {}

  std::string str() const { return to_iso_8601(m_micros); };

  int64_t micros() const { return m_micros; }

  static std::string to_iso_8601(int64_t);
  static int64_t from_iso_8601(const std::string&);

  static timestamp_t now();

 private:
  int64_t m_micros = 0;
};

}  // namespace krakpot
