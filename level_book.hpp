/* Copyright (C) 2024 John C. Finley - All rights reserved */
#pragma once

#include "constants.hpp"
#include "responses.hpp"

#include <boost/crc.hpp>

#include <map>
#include <unordered_map>

namespace krakpot {
namespace model {

using bid_side_t = std::map<price_t, qty_t, std::greater<price_t>>;
using ask_side_t = std::map<price_t, qty_t, std::less<price_t>>;

struct sides_t final {
  auto bids() const { return m_bids; }
  auto asks() const { return m_asks; }

  void accept_snapshot(const response::book_t &);
  void accept_update(const response::book_t &);

  uint64_t crc32() const;

  std::string str() const;

private:
  template <typename Q, typename S>
  void apply_update(const Q&, S&);

  void clear();
  void verify_checksum(uint64_t) const;

  template <typename S>
  boost::crc_32_type update_checksum(const boost::crc_32_type, const S &) const;

  bid_side_t m_bids;
  ask_side_t m_asks;
};


struct level_book_t final {
  using symbol_t = std::string;
  void accept(const response::book_t &);

  uint64_t crc32(symbol_t symbol) const;

  std::string str(std::string) const;

private:
  std::unordered_map<symbol_t, sides_t> m_sides;
};

template <typename S>
boost::crc_32_type sides_t::update_checksum(const boost::crc_32_type crc32,
                                            const S &side) const {
  auto result = crc32;
  auto depth = size_t{0};
  for (const auto &kv : side) {
    if (++depth > 10) {
      break;
    }
    const auto &price = kv.first;
    const auto &qty = kv.second;
    const auto trimmed_price = price.token().trimmed();
    const auto trimmed_qty = qty.token().trimmed();
    result.process_bytes(trimmed_price.data(), trimmed_price.size());
    result.process_bytes(trimmed_qty.data(), trimmed_qty.size());
  }
  return result;
}

template <typename Q, typename S>
void sides_t::apply_update(const Q& quotes, S& side) {
  for (const auto &quote : quotes) {
    const auto [price, qty] = quote;
    auto it = side.find(price);
    if (it != side.end()) {
      if (qty.value() == 0) {
        side.erase(it);
      } else {
        it->second = qty;
      }
    } else {
      side.insert(quote);
    }
  }
  if (side.size() > c_book_depth) {
    auto it = side.begin();
    std::advance(it, c_book_depth);
    side.erase(it, side.end());
  }
}

} // namespace model
} // namespace krakpot
