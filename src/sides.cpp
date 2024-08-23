#include "sides.hpp"

#include <boost/log/trivial.hpp>

#include <algorithm>
#include <string_view>

namespace kdr {
namespace model {

sides_t::sides_t(depth_t book_depth, integer_t price_precision,
                 integer_t qty_precision, const bid_side_t &bids,
                 const ask_side_t &asks)
    : m_book_depth{book_depth}, m_price_precision(price_precision),
      m_qty_precision(qty_precision), m_bids{bids}, m_asks{asks} {}

void sides_t::accept_snapshot(const response::book_t &snapshot) {
  clear();
  m_bids.insert(snapshot.bids().begin(), snapshot.bids().end());
  m_asks.insert(snapshot.asks().begin(), snapshot.asks().end());
  verify_checksum(snapshot.crc32());
}

void sides_t::accept_update(const response::book_t &update) {
  apply_update(update.bids(), m_bids);
  apply_update(update.asks(), m_asks);
  verify_checksum(update.crc32());
}

void sides_t::clear() {
  m_bids.clear();
  m_asks.clear();
}

void sides_t::verify_checksum(uint64_t expected_crc32) const {
  const auto actual_crc32 = crc32();
  if (expected_crc32 != actual_crc32) {
    const auto message =
        "bogus crc32 expected: " + std::to_string(expected_crc32) +
        " actual: " + std::to_string(actual_crc32);
    throw std::runtime_error(message);
  }
}

uint64_t sides_t::crc32() const {
  boost::crc_32_type result;
  result = update_checksum(result, asks());
  result = update_checksum(result, bids());
  return result.checksum();
}

boost::json::object sides_t::to_json_obj() const {
  auto bid_objs = boost::json::array{};
  std::transform(
      bids().begin(), bids().end(), std::back_inserter(bid_objs),
      [this](const auto &kv) {
        const boost::json::object quote{
            {kv.first.str(price_precision()), kv.second.str(qty_precision())}};
        return quote;
      });

  auto ask_objs = boost::json::array{};
  std::transform(
      asks().begin(), asks().end(), std::back_inserter(ask_objs),
      [this](const auto &kv) {
        const boost::json::object quote{
            {kv.first.str(price_precision()), kv.second.str(qty_precision())}};
        return quote;
      });

  const boost::json::object result = {{response::book_t::c_bids, bid_objs},
                                      {response::book_t::c_asks, ask_objs}};
  return result;
}

} // namespace model
} // namespace kdr
