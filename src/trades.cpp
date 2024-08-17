#include "trades.hpp"

#include <algorithm>
#include <array>
#include <chrono>

namespace kdr {
namespace response {

trades_t trades_t::from_json(simdjson::ondemand::document& response) {
  auto result = trades_t{};
  auto buffer = std::string_view{};

  buffer = response[header_t::c_channel].get_string();
  const auto channel = std::string{buffer.begin(), buffer.end()};
  buffer = response[header_t::c_type].get_string();
  const auto type = std::string{buffer.begin(), buffer.end()};
  result.m_header = header_t{timestamp_t::now(), channel, type};

  for (simdjson::fallback::ondemand::object obj : response[c_response_data]) {
    const auto trade = model::trade_t::from_json(obj);
    result.m_trades.push_back(trade);
  }

  return result;
}

boost::json::object trades_t::to_json_obj(integer_t price_precision,
                                          integer_t qty_precision) const {
  auto trades = boost::json::array();
  std::transform(
      m_trades.begin(), m_trades.end(), std::back_inserter(trades),
      [price_precision, qty_precision](const model::trade_t& trade) {
        const boost::json::object result = {
            {model::trade_t::c_ord_type, trade.ord_type()},
            {model::trade_t::c_price, trade.price().str(price_precision)},
            {model::trade_t::c_qty, trade.qty().str(qty_precision)},
            {model::trade_t::c_side, trade.side()},
            {model::trade_t::c_symbol, trade.symbol()},
            {model::trade_t::c_timestamp, trade.timestamp().str()},
            {model::trade_t::c_trade_id, trade.trade_id()},
        };
        return result;
      });

  const boost::json::object result = {{header_t::c_channel, m_header.channel()},
                                      {c_response_data, trades},
                                      {header_t::c_type, m_header.type()}};
  return result;
}

}  // namespace response
}  // namespace kdr
