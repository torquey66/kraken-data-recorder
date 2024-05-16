/* Copyright (C) 2024 John C. Finley - All rights reserved */

#include "requests.hpp"

#include <nlohmann/json.hpp>

namespace krakpot {
namespace request {

nlohmann::json ping_t::to_json() const {
  const nlohmann::json result = {{c_request_method, c_method_ping},
                                 {c_request_req_id, m_req_id}};
  return result;
}

nlohmann::json subscribe_instrument_t::to_json() const {
  const nlohmann::json result = {{c_request_method, c_method_subscribe},
                                 {c_request_params,
                                  {{c_request_channel, c_channel_instrument},
                                   {c_param_snapshot, m_snapshot}}},
                                 {c_request_req_id, m_req_id}};
  return result;
}

nlohmann::json subscribe_book_t::to_json() const {
  const nlohmann::json result = {{c_request_method, c_method_subscribe},
                                 {c_request_params,
                                  {{c_request_channel, c_channel_book},
                                   {c_param_depth, m_depth},
                                   {c_param_snapshot, m_snapshot},
                                   {c_param_symbol, m_symbols}}},
                                 {c_request_req_id, m_req_id}};
  return result;
}

nlohmann::json subscribe_trade_t::to_json() const {
  const nlohmann::json result = {{c_request_method, c_method_subscribe},
                                 {c_request_params,
                                  {
                                      {c_request_channel, c_channel_trade},
                                      {c_param_snapshot, m_snapshot},
                                      {c_param_symbol, m_symbols},
                                  }},
                                 {c_request_req_id, m_req_id}};
  return result;
}

} // namespace request
} // namespace krakpot
