/* Copyright (C) 2024 John C. Finley - All rights reserved */
#include "level_book.hpp"

#include <boost/log/trivial.hpp>

#include <algorithm>
#include <string_view>

namespace krakpot {
namespace model {

void sides_t::accept_snapshot(const response::book_t &snapshot) {
  clear();
  m_bids.insert(snapshot.bids().begin(), snapshot.bids().end());
  m_asks.insert(snapshot.asks().begin(), snapshot.asks().end());
  verify_checksum(snapshot.crc32());
}

void sides_t::accept_update(const response::book_t &update) {
  apply_update(update.bids(), m_bids);
  apply_update(update.asks(), m_asks);
  verify_checksum(update.crc32());
}

void sides_t::clear() {
  m_bids.clear();
  m_asks.clear();
}

void sides_t::verify_checksum(uint64_t expected_crc32) const {
  boost::crc_32_type result;
  result = update_checksum(result, asks());
  result = update_checksum(result, bids());

  const auto actual_crc32 = result.checksum();
  if (expected_crc32 != actual_crc32) {
    throw std::runtime_error(
        "bogus crc32 expected: " + std::to_string(expected_crc32) +
        " actual: " + std::to_string(actual_crc32));
  }
}

void level_book_t::accept(const response::book_t &response) {
  auto &sides = m_sides[response.symbol()];
  const auto type = response.header().type();
  if (type == c_book_type_snapshot) {
    return sides.accept_snapshot(response);
  }
  if (type == c_book_type_update) {
    return sides.accept_update(response);
  }
  throw std::runtime_error("bogus book channel type: '" + type + "'");
}

} // namespace model
} // namespace krakpot
