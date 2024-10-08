#pragma once

#include "header.hpp"
#include "types.hpp"

#include <boost/json.hpp>
#include <simdjson.h>

#include <string>
#include <string_view>
#include <vector>

namespace kdr {
namespace response {

/**
 * https://docs.kraken.com/websockets-v2/#book
 */
struct book_t final {
  using asks_t = std::vector<ask_t>;
  using bids_t = std::vector<bid_t>;

  /** Field names */
  static const std::string_view c_asks;
  static const std::string_view c_bids;
  static const std::string_view c_checksum;
  static const std::string_view c_price;
  static const std::string_view c_qty;
  static const std::string_view c_side;
  static const std::string_view c_symbol;
  static const std::string_view c_timestamp;

  /** Field values */
  static const std::string_view c_snapshot;
  static const std::string_view c_update;

  book_t() = default;
  book_t(const header_t &header, const asks_t &asks, const bids_t &bids,
         uint64_t crc32, std::string symbol, timestamp_t timestamp);

  const header_t &header() const { return m_header; }
  const bids_t &bids() const { return m_bids; }
  const asks_t &asks() const { return m_asks; }
  uint64_t crc32() const { return m_crc32; }
  const std::string &symbol() const { return m_symbol; }
  timestamp_t timestamp() const { return m_timestamp; }

  static book_t from_json(simdjson::ondemand::document &response);

  boost::json::object to_json_obj(integer_t price_precision,
                                  integer_t qty_precision) const;
  std::string str(integer_t price_precision, integer_t qty_precision) const {
    return boost::json::serialize(to_json_obj(price_precision, qty_precision));
  }

private:
  header_t m_header;
  asks_t m_asks;
  bids_t m_bids;
  uint64_t m_crc32 = 0;
  std::string m_symbol;
  timestamp_t m_timestamp;
};

} // namespace response
} // namespace kdr
