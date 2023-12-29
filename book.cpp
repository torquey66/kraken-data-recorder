#include "book.hpp"

#include <stdexcept>

namespace krakpot {

book_t::book_t(const record_t &record) {
  if (record.as.empty() || record.bs.empty() || !record.a.empty() ||
      !record.b.empty() || record.c != 0) {
    throw std::runtime_error("bad snapshot record");
  }
  for (const auto &entry : record.as) {
    // m_asks.emplace(std::piecewise_construct,
    // std::forward_as_tuple(entry.price),
    //                std::forward_as_tuple(entry.volume, entry.timestamp));
    m_asks.try_emplace(entry.price, entry.volume, entry.timestamp);
  }
}

void book_t::update(const record_t &record) {
  for (const auto &entry : record.b) {
    if (entry.volume == 0) {
      m_bids.erase(entry.price);
    } else {
      m_bids[entry.price] = book_entry_t{entry.volume, entry.timestamp};
    }
  }
  for (const auto &entry : record.a) {
    if (entry.volume == 0) {
      m_asks.erase(entry.price);
    } else {
      m_asks[entry.price] = book_entry_t{entry.volume, entry.timestamp};
    }
  }
}

} // namespace krakpot
