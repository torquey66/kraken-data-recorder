#pragma once

#include <boost/log/trivial.hpp>
#include <simdjson.h>

#include <string>

namespace krakpot {

using channel_id_t = int64_t;
using channel_name_t = std::string;
using checksum_t = std::string;
using pair_t = std::string;
using update_type_t = std::string;

/* !@# Placeholders for better decimal types. */
using price_t = double;
using timestamp_t = double;
using volume_t = double;

static constexpr channel_id_t BAD_CHANNEL_ID = -1;

struct entry_t final {
  price_t price = 0.;
  volume_t volume = 0.;
  timestamp_t timestamp = 0.;
  update_type_t update_type;
};

struct record_t final {
  channel_id_t channel_id = BAD_CHANNEL_ID;
  std::vector<entry_t> as;
  std::vector<entry_t> bs;
  std::vector<entry_t> a;
  std::vector<entry_t> b;
  checksum_t c;
  channel_name_t channel_name;
  pair_t pair;

  void reset();

  std::string to_string() const;
};

struct processor_t final {

  void process(std::string &);

private:
  simdjson::error_code process_book_msg(simdjson::ondemand::document &);

  simdjson::ondemand::parser m_parser;
  record_t m_record;
};

} // namespace krakpot
