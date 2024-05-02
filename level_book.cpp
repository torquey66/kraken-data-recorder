/* Copyright (C) 2024 John C. Finley - All rights reserved */
#include "level_book.hpp"

#include <boost/crc.hpp>
#include <boost/log/trivial.hpp>

#include <algorithm>
#include <string_view>

namespace {

boost::crc_32_type process(const krakpot::decimal_t &value,
                           boost::crc_32_type crc32) {
  auto result = crc32;
  const auto &token = value.token();
  const auto it =
      std::find_if_not(token.begin(), token.end(),
                       [](const char ch) { return ch == '0' || ch == '.'; });
  const auto offset = std::distance(token.begin(), it);
  const auto data = token.data() + offset;
  const auto length = token.size() - offset;
  result.process_bytes(data, length);
  return result;
}

} // anonymous namespace

namespace krakpot {
namespace model {

void sides_t::accept_snapshot(const response::book_t &snapshot) {
  clear();
  m_bids.insert(snapshot.bids().begin(), snapshot.bids().end());
  m_asks.insert(snapshot.asks().begin(), snapshot.asks().end());
}

void sides_t::accept_update(const response::book_t &update) {
  apply_update(update.bids(), m_bids);
  apply_update(update.asks(), m_asks);
}

void sides_t::clear() {
  m_bids.clear();
  m_asks.clear();
}

void level_book_t::accept(const response::book_t &response) {
  auto &sides = m_sides[response.symbol()];
  const auto type = response.header().type();
  if (type == c_book_type_snapshot) {
    sides.accept_snapshot(response);
    const auto expected_cksum = response.crc32();
    const auto actual_cksum = crc32(response.symbol());
    if (expected_cksum != actual_cksum) {
      throw std::runtime_error(
          "bogus cksum expected: " + std::to_string(expected_cksum) +
          " actual: " + std::to_string(actual_cksum));
    }
    return;
  }
  if (type == c_book_type_update) {
    return sides.accept_update(response);
  }
  throw std::runtime_error("bogus book channel type: '" + type + "'");
}

uint64_t level_book_t::crc32(std::string symbol) const {
  boost::crc_32_type result;

  const auto sit = m_sides.find(symbol);
  if (sit == m_sides.end()) {
    throw std::runtime_error("bogus symbol: '" + symbol + "'");
  }
  const auto &side = sit->second;

  // TODO eliminate duplicate code

  auto depth = size_t{0};
  for (const auto &kv : side.asks()) {
    if (++depth > 10) {
      break;
    }
    const auto &price = kv.first;
    const auto &qty = kv.second;
    result = process(price, result);
    result = process(qty, result);
  }

  depth = 0;
  for (const auto &kv : side.bids()) {
    if (++depth > 10) {
      break;
    }
    const auto &price = kv.first;
    const auto &qty = kv.second;
    result = process(price, result);
    result = process(qty, result);
  }

  return result.checksum();
}

} // namespace model
} // namespace krakpot
