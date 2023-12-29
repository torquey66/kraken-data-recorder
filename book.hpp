#pragma once

#include "record.hpp"

#include <algorithm>
#include <limits>
#include <map>

namespace krakpot {

enum Side { Ask = 0, Bid = 1 };

struct book_t final {

  struct book_entry_t final {
    explicit book_entry_t() {}
    book_entry_t(volume_t in_volume, timestamp_t in_timestamp)
        : volume(in_volume), timestamp(in_timestamp) {}

    volume_t volume = std::numeric_limits<volume_t>::signaling_NaN();
    timestamp_t timestamp = std::numeric_limits<timestamp_t>::signaling_NaN();
  };

  explicit book_t() {}
  book_t(const record_t &);

  void update(const record_t &);

private:
  using bids_t = std::map<price_t, book_entry_t, std::greater<price_t>>;
  using asks_t = std::map<price_t, book_entry_t, std::less<price_t>>;

  bids_t m_bids;
  asks_t m_asks;
};

} // namespace krakpot
