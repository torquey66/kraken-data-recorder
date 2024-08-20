#pragma once

#include "book.hpp"
#include "constants.hpp"
#include "depth.hpp"
#include "pair.hpp"
#include "types.hpp"

#include <boost/crc.hpp>
#include <boost/json.hpp>

#include <map>
#include <unordered_map>

namespace kdr {
namespace model {

using bid_side_t = std::map<price_t, qty_t, std::greater<price_t>>;
using ask_side_t = std::map<price_t, qty_t, std::less<price_t>>;

struct sides_t final {
  sides_t(depth_t, integer_t price_precision, integer_t qty_precision,
          const bid_side_t &, const ask_side_t &);

  depth_t book_depth() const { return m_book_depth; }
  integer_t price_precision() const { return m_price_precision; }
  integer_t qty_precision() const { return m_qty_precision; }

  const bid_side_t &bids() const { return m_bids; }
  const ask_side_t &asks() const { return m_asks; }

  void accept_snapshot(const response::book_t &);
  void accept_update(const response::book_t &);

  uint64_t crc32() const;

  boost::json::object to_json_obj() const;
  std::string str() const { return boost::json::serialize(to_json_obj()); }

private:
  template <typename Q, typename S> void apply_update(const Q &, S &);

  void clear();
  void verify_checksum(uint64_t) const;

  template <typename S>
  boost::crc_32_type update_checksum(const boost::crc_32_type, const S &) const;

  depth_t m_book_depth;
  integer_t m_price_precision = 0;
  integer_t m_qty_precision = 0;

  bid_side_t m_bids;
  ask_side_t m_asks;

  std::string m_price_fmt;
  std::string m_side_fmt;
};

struct level_book_t final {
  using symbol_t = std::string;

  const sides_t &sides(symbol_t) const;

  level_book_t(depth_t book_depth) : m_book_depth{book_depth} {};

  void accept(const model::pair_t &);
  void accept(const response::book_t &);

  uint64_t crc32(symbol_t symbol) const;

  std::string str(std::string) const;

private:
  depth_t m_book_depth;
  std::unordered_map<symbol_t, sides_t> m_sides;
};

template <typename S>
boost::crc_32_type sides_t::update_checksum(boost::crc_32_type crc32,
                                            const S &side) const {
  static constexpr size_t c_crc32_depth = 10;
  auto &result = crc32;
  auto depth = size_t{0};
  for (const auto &kv : side) {
    if (++depth > c_crc32_depth) {
      break;
    }
    const auto &price = kv.first;
    const auto &qty = kv.second;
    price.process(crc32, price_precision());
    qty.process(crc32, qty_precision());
  }
  return result;
}

template <typename Q, typename S>
void sides_t::apply_update(const Q &quotes, S &side) {
  for (const auto &quote : quotes) {
    const auto &[price, qty] = quote;
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
  if (side.size() > static_cast<size_t>(m_book_depth)) {
    auto it = side.begin();
    std::advance(it, m_book_depth);
    side.erase(it, side.end());
  }
}

} // namespace model
} // namespace kdr
