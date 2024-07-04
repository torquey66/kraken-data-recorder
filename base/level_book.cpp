/* Copyright (C) 2024 John C. Finley - All rights reserved */
#include "level_book.hpp"

#include <boost/log/trivial.hpp>

#include <algorithm>
#include <string_view>

namespace krakpot {
namespace model {

sides_t::sides_t(depth_t book_depth,
                 integer_t price_precision,
                 integer_t qty_precision,
                 const bid_side_t& bids,
                 const ask_side_t& asks)
    : m_book_depth{book_depth},
      m_price_precision(price_precision),
      m_qty_precision(qty_precision),
      m_bids{bids},
      m_asks{asks} {}

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
  const auto actual_crc32 = crc32();
  if (expected_crc32 != actual_crc32) {
    const auto message =
        "bogus crc32 expected: " + std::to_string(expected_crc32) +
        " actual: " + std::to_string(actual_crc32);
    throw std::runtime_error(message);
  }
}

uint64_t sides_t::crc32() const {
  boost::crc_32_type result;
  result = update_checksum(result, asks());
  result = update_checksum(result, bids());
  return result.checksum();
}

nlohmann::json sides_t::to_json() const {
  auto bids_json = nlohmann::json{};
  for (const auto& bid : bids()) {
    const nlohmann::json quote{
        {bid.first.str(price_precision()), bid.second.str(qty_precision())}};
    bids_json.push_back(quote);
  }

  auto asks_json = nlohmann::json{};
  for (const auto& ask : asks()) {
    const nlohmann::json quote{
        {ask.first.str(price_precision()), ask.second.str(qty_precision())}};
    asks_json.push_back(quote);
  }

  const nlohmann::json result = {{c_book_bids, bids_json},
                                 {c_book_asks, asks_json}};
  return result;
}

const sides_t &level_book_t::sides(symbol_t symbol) const {
  const auto it = m_sides.find(symbol);
  if (it == m_sides.end()) {
    const auto message = "unknown symbol: " + symbol;
    throw std::runtime_error(message);
  }
  return it->second;
}

void level_book_t::accept(const response::pair_t& pair) {
  auto it = m_sides.find(pair.symbol());
  if (it == m_sides.end()) {
    // The expected case: we receive a pair for which we have not
    // created sides.
    auto sides = sides_t{m_book_depth, pair.price_precision(),
                         pair.qty_precision(), bid_side_t{}, ask_side_t{}};
    m_sides.emplace(std::make_pair(pair.symbol(), sides));
  } else {
    // The less expected case: we receive a pair response for a symbol
    // we have already seen. In this case, we only need to replacd it
    // if the precisions have changed.
    const auto& existing_sides = it->second;
    if (pair.price_precision() != existing_sides.price_precision() ||
        pair.qty_precision() != existing_sides.qty_precision()) {
      auto new_sides =
          sides_t{m_book_depth, pair.price_precision(), pair.qty_precision(),
                  existing_sides.bids(), existing_sides.asks()};
      it->second = new_sides;
    }
  }
}

void level_book_t::accept(const response::book_t& book) {
  auto it = m_sides.find(book.symbol());
  if (it == m_sides.end()) {
    const auto message = "unknown symbol: " + book.symbol();
    throw std::runtime_error(message);
  }
  auto& sides = it->second;
  const auto type = book.header().type();
  if (type == c_book_type_snapshot) {
    return sides.accept_snapshot(book);
  }
  if (type == c_book_type_update) {
    return sides.accept_update(book);
  }
  throw std::runtime_error("bogus book channel type: '" + type + "'");
}

uint64_t level_book_t::crc32(symbol_t symbol) const {
  const auto it = m_sides.find(symbol);
  if (it == m_sides.end()) {
    throw std::runtime_error("bogus symbol: " + symbol);
  }
  const auto& sides = it->second;
  return sides.crc32();
}

std::string level_book_t::str(std::string symbol) const {
  const auto& side = m_sides.at(symbol);
  const nlohmann::json result{{c_book_symbol, symbol},
                              {c_book_side, side.str()}};
  return result.dump(3);
}

}  // namespace model
}  // namespace krakpot
