#include "level_book.hpp"

#include <boost/log/trivial.hpp>

namespace kdr {
namespace model {

const sides_t &level_book_t::sides(symbol_t symbol) const {
  const auto it = m_sides.find(symbol);
  if (it == m_sides.end()) {
    const auto message = "unknown symbol: " + symbol;
    throw std::runtime_error(message);
  }
  return it->second;
}

void level_book_t::accept(const model::pair_t &pair) {
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
    const auto &existing_sides = it->second;
    if (pair.price_precision() != existing_sides.price_precision() ||
        pair.qty_precision() != existing_sides.qty_precision()) {
      auto new_sides =
          sides_t{m_book_depth, pair.price_precision(), pair.qty_precision(),
                  existing_sides.bids(), existing_sides.asks()};
      it->second = new_sides;
    }
  }
}

void level_book_t::accept(const response::book_t &book) {
  auto it = m_sides.find(book.symbol());
  if (it == m_sides.end()) {
    const auto message = "unknown symbol: " + book.symbol();
    throw std::runtime_error(message);
  }
  auto &sides = it->second;
  const auto type = book.header().type();
  if (type == response::book_t::c_snapshot) {
    return sides.accept_snapshot(book);
  }
  if (type == response::book_t::c_update) {
    return sides.accept_update(book);
  }
  throw std::runtime_error("bogus book channel type: '" + type + "'");
}

uint64_t level_book_t::crc32(symbol_t symbol) const {
  const auto it = m_sides.find(symbol);
  if (it == m_sides.end()) {
    throw std::runtime_error("bogus symbol: " + symbol);
  }
  const auto &sides = it->second;
  return sides.crc32();
}

std::string level_book_t::str(std::string symbol) const {
  const auto &side = m_sides.at(symbol);
  const boost::json::object result{{response::book_t::c_symbol, symbol},
                                   {response::book_t::c_side, side.str()}};
  return boost::json::serialize(result);
}

} // namespace model
} // namespace kdr
