#pragma once

#include "asset.hpp"
#include "pair.hpp"

#include "nlohmann/json.hpp"
#include <simdjson.h>

#include <string>
#include <vector>

namespace krakpot {
namespace response {

/**
 */
struct header_t final {
  header_t() = default;
  header_t(std::string channel, std::string type)
      : m_channel(channel), m_type(type) {}

  const std::string &channel() const { return m_channel; }
  const std::string &type() const { return m_type; }

private:
  std::string m_channel;
  std::string m_type;
};

/**
 */
struct instrument_t final {
  instrument_t() = default;

  static instrument_t from_json(simdjson::ondemand::document &);

  nlohmann::json to_json() const;
  std::string str() const { return to_json().dump(); }

  const std::vector<asset_t> &assets() const { return m_assets; }
  const std::vector<pair_t> &pairs() const { return m_pairs; }

private:
  header_t m_header;
  std::vector<asset_t> m_assets;
  std::vector<pair_t> m_pairs;
};

/**
 */
struct book_t final {
  // !@# TODO: consider moving these to a central "types" header
  using price_t = double;
  using qty_t = double;
  using quote_t = std::pair<price_t, qty_t>;
  using ask_t = quote_t;
  using bid_t = quote_t;

  book_t() = default;

private:
  using asks_t = std::vector<bid_t>;
  using bids_t = std::vector<bid_t>;

  header_t m_header;
  asks_t m_asks;
  bids_t m_bids;
  uint32_t m_crc32;
  std::string m_symbol;
  std::string
      m_tm; // RFC3339 - !@# TODO: consider xlating to binary representation
};

} // namespace response
} // namespace krakpot
