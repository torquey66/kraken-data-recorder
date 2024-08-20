#include "requests.hpp"

#include <algorithm>

namespace kdr {
namespace request {

boost::json::object ping_t::to_json_obj() const {
  const boost::json::object result = {{c_request_method, c_method_ping},
                                      {c_request_req_id, m_req_id}};
  return result;
}

boost::json::object subscribe_instrument_t::to_json_obj() const {
  const boost::json::object result = {
      {c_request_method, c_method_subscribe},
      {c_request_params,
       {{c_request_channel, c_channel_instrument},
        {c_param_snapshot, m_snapshot}}},
      {c_request_req_id, m_req_id}};
  return result;
}

boost::json::object subscribe_book_t::to_json_obj() const {
  auto symbol_objs = boost::json::array{};
  std::transform(
      m_symbols.begin(), m_symbols.end(), std::back_inserter(symbol_objs),
      [](const std::string &symbol) { return boost::json::string{symbol}; });
  const boost::json::object result = {{c_request_method, c_method_subscribe},
                                      {c_request_params,
                                       {{c_request_channel, c_channel_book},
                                        {c_param_depth, m_depth},
                                        {c_param_snapshot, m_snapshot},
                                        {c_param_symbol, symbol_objs}}},
                                      {c_request_req_id, m_req_id}};
  return result;
}

boost::json::object subscribe_trade_t::to_json_obj() const {
  auto symbol_objs = boost::json::array{};
  std::transform(
      m_symbols.begin(), m_symbols.end(), std::back_inserter(symbol_objs),
      [](const std::string &symbol) { return boost::json::string{symbol}; });
  const boost::json::object result = {{c_request_method, c_method_subscribe},
                                      {c_request_params,
                                       {{c_request_channel, c_channel_trade},
                                        {c_param_snapshot, m_snapshot},
                                        {c_param_symbol, symbol_objs}}},
                                      {c_request_req_id, m_req_id}};
  return result;
}

} // namespace request
} // namespace kdr
