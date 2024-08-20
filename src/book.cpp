#include "book.hpp"

#include <algorithm>
#include <array>

namespace {

template <typename O>
kdr::decimal_t extract_decimal(O &obj, std::string_view field) {
  const auto token = std::string_view{obj[field].raw_json_token()};
  const auto result = kdr::decimal_t{token};
  return result;
}

} // namespace

namespace kdr {
namespace response {

static constexpr auto c_asks_field = std::to_array("asks");
static constexpr auto c_bids_field = std::to_array("bids");
static constexpr auto c_checksum_field = std::to_array("checksum");
static constexpr auto c_price_field = std::to_array("price");
static constexpr auto c_qty_field = std::to_array("qty");
static constexpr auto c_side_field = std::to_array("side");
static constexpr auto c_symbol_field = std::to_array("symbol");
static constexpr auto c_timestamp_field = std::to_array("timestamp");

const std::string_view book_t::c_asks{c_asks_field.data(),
                                      c_asks_field.size() - 1};
const std::string_view book_t::c_bids{c_bids_field.data(),
                                      c_bids_field.size() - 1};
const std::string_view book_t::c_checksum{c_checksum_field.data(),
                                          c_checksum_field.size() - 1};
const std::string_view book_t::c_price{c_price_field.data(),
                                       c_price_field.size() - 1};
const std::string_view book_t::c_qty{c_qty_field.data(),
                                     c_qty_field.size() - 1};
const std::string_view book_t::c_side{c_side_field.data(),
                                      c_side_field.size() - 1};
const std::string_view book_t::c_symbol{c_symbol_field.data(),
                                        c_symbol_field.size() - 1};
const std::string_view book_t::c_timestamp{c_timestamp_field.data(),
                                           c_timestamp_field.size() - 1};

static constexpr auto c_snapshot_value = std::to_array("snapshot");
static constexpr auto c_update_value = std::to_array("update");

const std::string_view book_t::c_snapshot{c_snapshot_value.data(),
                                          c_snapshot_value.size() - 1};
const std::string_view book_t::c_update{c_update_value.data(),
                                        c_update_value.size() - 1};

book_t::book_t(const header_t &header, const asks_t &asks, const bids_t &bids,
               uint64_t crc32, std::string symbol, timestamp_t timestamp)
    : m_header(header), m_asks(asks), m_bids(bids), m_crc32(crc32),
      m_symbol(std::move(symbol)), m_timestamp(timestamp) {}

book_t book_t::from_json(simdjson::ondemand::document& response) {
  auto result = book_t{};
  auto buffer = std::string_view{};

  buffer = response[header_t::c_channel].get_string();
  const auto channel = std::string{buffer.begin(), buffer.end()};
  buffer = response[header_t::c_type].get_string();
  const auto type = std::string{buffer.begin(), buffer.end()};
  result.m_header = header_t{timestamp_t::now(), channel, type};

  // TODO: it is entirely unclear to me why `data` is an array since
  // it only ever seems to contain a single entry.
  bool processed = false;
  for (auto data : response[c_response_data]) {
    if (processed) {
      throw std::runtime_error(
          "TODO: fix problem with multiple data[] entries");
    }
    processed = true;

    // TODO: eliminate duplication
    for (simdjson::fallback::ondemand::object obj : data[c_asks]) {
      const auto price = extract_decimal(obj, c_price);
      const auto qty = extract_decimal(obj, c_qty);
      const ask_t ask = std::make_pair(price, qty);
      result.m_asks.push_back(ask);
    }

    for (simdjson::fallback::ondemand::object obj : data[c_bids]) {
      const auto price = extract_decimal(obj, c_price);
      const auto qty = extract_decimal(obj, c_qty);
      const bid_t bid = std::make_pair(price, qty);
      result.m_bids.push_back(bid);
    }

    result.m_crc32 = data[c_checksum].get_uint64();

    buffer = data[c_symbol].get_string();
    const auto symbol = std::string{buffer.begin(), buffer.end()};
    result.m_symbol = symbol;

    if (type == c_update) {
      buffer = data[c_timestamp].get_string();
      result.m_timestamp =
          timestamp_t::from_iso_8601(std::string{buffer.data(), buffer.size()});
    }
  }

  return result;
}

boost::json::object book_t::to_json_obj(integer_t price_precision,
                                        integer_t qty_precision) const {
  auto asks = boost::json::array();
  std::transform(m_asks.begin(), m_asks.end(), std::back_inserter(asks),
                 [price_precision, qty_precision](const ask_t& ask) {
                   const boost::json::object result = {
                       {c_price, ask.first.double_value(price_precision)},
                       {c_qty, ask.second.double_value(qty_precision)}};
                   return result;
                 });

  auto bids = boost::json::array();
  std::transform(m_bids.begin(), m_bids.end(), std::back_inserter(bids),
                 [price_precision, qty_precision](const bid_t& bid) {
                   const boost::json::object result = {
                       {c_price, bid.first.double_value(price_precision)},
                       {c_qty, bid.second.double_value(qty_precision)}};
                   return result;
                 });

  auto content = boost::json::object{};
  content[c_asks] = asks;
  content[c_bids] = bids;
  content[c_checksum] = m_crc32;
  content[c_symbol] = m_symbol;
  content[c_timestamp] = m_timestamp.str();

  auto data = boost::json::array();
  data.push_back(content);

  const boost::json::object result = {{header_t::c_channel, m_header.channel()},
                                      {c_response_data, data},
                                      {header_t::c_type, m_header.type()}};

  return result;
}

}  // namespace response
}  // namespace kdr
