/* Copyright (C) 2024 John C. Finley - All rights reserved */

#include "book.hpp"

#include <boost/log/trivial.hpp> // !@#

#include <cstdlib>
#include <stdexcept>

namespace krakpot {

book_t::book_t(const record_t &record) {
  if (record.as.empty() || record.bs.empty() || !record.a.empty() ||
      !record.b.empty() || record.c != 0) {
    throw std::runtime_error("bad snapshot record");
  }
  for (const auto &entry : record.as) {
    m_asks.try_emplace(entry.price, entry.volume, entry.timestamp);
  }
}

void book_t::update(const record_t &record) {
  for (const auto &entry : record.b) {
    if (entry.volume.as_double() == 0.) {
      m_bids.erase(entry.price);
    } else {
      m_bids[entry.price] = book_entry_t{entry.volume, entry.timestamp};
    }
  }
  for (const auto &entry : record.a) {
    if (entry.volume.as_double() == 0.) {
      m_asks.erase(entry.price);
    } else {
      m_asks[entry.price] = book_entry_t{entry.volume, entry.timestamp};
    }
  }
}

} // namespace krakpot
