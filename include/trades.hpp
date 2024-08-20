#pragma once

#include "header.hpp"
#include "trade.hpp"
#include "types.hpp"

#include <vector>

namespace kdr {
namespace response {

/**
 * https://docs.kraken.com/websockets-v2/#trade
 */
struct trades_t final {
  trades_t() = default;

  const header_t &header() const { return m_header; }

  static trades_t from_json(simdjson::ondemand::document &);

  boost::json::object to_json_obj(integer_t price_precision,
                                  integer_t qty_precision) const;
  std::string str(integer_t price_precision, integer_t qty_precision) const {
    return boost::json::serialize(to_json_obj(price_precision, qty_precision));
  }

  auto begin() const { return m_trades.begin(); }
  auto end() const { return m_trades.end(); }
  auto size() const { return m_trades.size(); }

private:
  header_t m_header;
  std::vector<model::trade_t> m_trades;
};

} // namespace response
} // namespace kdr
