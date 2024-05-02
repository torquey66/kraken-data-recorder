/* Copyright (C) 2024 John C. Finley - All rights reserved */
#pragma once

#include "constants.hpp"
#include "responses.hpp"

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

private:
  template <typename Q, typename S>
  void apply_update(const Q&, S&);
  
  void clear();

  bid_side_t m_bids;
  ask_side_t m_asks;
};


struct level_book_t final {
  void accept(const response::book_t &);

private:
  using symbol_t = std::string;
  std::unordered_map<symbol_t, sides_t> m_sides;
};


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
