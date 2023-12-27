#pragma once

#include <nlohmann/json.hpp>

#include <map>

namespace krakpot {

using price_t = double; // !@# temporary
using volume_t = double; // !@# temporary
using timestamp_t = double; // !@# temporary

enum Side { Ask = 0, Bid = 1 };

struct entry_t final {
  volume_t volume;
  timestamp_t tm;
};

template <typename Compare>
struct side_t final {

private:
  std::map<price_t, entry_t, Compare> m_levels;
};

struct book_t final {

  book_t(const nlohmann::json snapshot);

  void update(const nlohmann::json edits);
};

} // namespace krakpot
