#pragma once

#include "decimal.hpp"

#include <string>
#include <vector>

namespace krakpot {

using channel_id_t = int64_t;
using channel_name_t = std::string;
using checksum_t = uint64_t;
using pair_t = std::string;
using update_type_t = std::string;

/* !@# Placeholders for better decimal types. */
using price_t = decimal_t;
using timestamp_t = std::string;
using volume_t = decimal_t;

static constexpr channel_id_t BAD_CHANNEL_ID = -1;

struct entry_t final {
  price_t price;
  volume_t volume;
  timestamp_t timestamp;
  update_type_t update_type;
};

struct record_t final {
  channel_id_t channel_id = BAD_CHANNEL_ID;
  std::vector<entry_t> as;
  std::vector<entry_t> bs;
  std::vector<entry_t> a;
  std::vector<entry_t> b;
  checksum_t c = 0;
  channel_name_t channel_name;
  pair_t pair;

  void reset();

  bool is_snapshot() const { return !as.empty() && !bs.empty(); }

  std::string to_string() const;
};

} // namespace krakpot
