/* Copyright (C) 2024 John C. Finley - All rights reserved */

#include "pair.hpp"

namespace krakpot {
namespace response {

const std::unordered_map<std::string, pair_t::status_t>
    pair_t::c_str_to_status = {
        {c_pair_status_cancel_only, e_cancel_only},
        {c_pair_status_delisted, e_delisted},
        {c_pair_status_limit_only, e_limit_only},
        {c_pair_status_maintenance, e_maintenance},
        {c_pair_status_online, e_online},
        {c_pair_status_post_only, e_post_only},
        {c_pair_status_reduce_only, e_reduce_only},
        {c_pair_status_work_in_progress, e_work_in_progress},
};

const std::unordered_map<pair_t::status_t, std::string>
    pair_t::c_status_to_str = {
        {e_invalid, c_pair_status_invalid},
        {e_cancel_only, c_pair_status_cancel_only},
        {e_delisted, c_pair_status_delisted},
        {e_limit_only, c_pair_status_limit_only},
        {e_maintenance, c_pair_status_maintenance},
        {e_online, c_pair_status_online},
        {e_post_only, c_pair_status_post_only},
        {e_reduce_only, c_pair_status_reduce_only},
        {e_work_in_progress, c_pair_status_work_in_progress},
};

pair_t pair_t::from_json(simdjson::ondemand::object &pair_obj) {
  auto result = pair_t{};
  auto buffer = std::string_view{};
  simdjson::ondemand::value optional_val{};

  buffer = pair_obj[c_pair_base].get_string();
  result.m_base = std::string{buffer.begin(), buffer.end()};

  result.m_cost_min = pair_obj[c_pair_cost_min].get_double();
  result.m_cost_precision = pair_obj[c_pair_cost_precision].get_int64();
  result.m_has_index = pair_obj[c_pair_has_index].get_bool();

  if (pair_obj[c_pair_margin_initial].get(optional_val) == simdjson::SUCCESS) {
    result.m_margin_initial = optional_val.get_double();
  }

  result.m_marginable = pair_obj[c_pair_marginable].get_bool();

  if (pair_obj[c_pair_position_limit_long].get(optional_val) == simdjson::SUCCESS) {
    result.m_position_limit_long = optional_val.get_int64();
  }
  if (pair_obj[c_pair_position_limit_short].get(optional_val) == simdjson::SUCCESS) {
    result.m_position_limit_short = optional_val.get_int64();
  }

  result.m_price_increment = pair_obj[c_pair_price_increment].get_double();
  result.m_price_precision = pair_obj[c_pair_price_precision].get_int64();
  result.m_qty_increment = pair_obj[c_pair_qty_increment].get_double();
  result.m_qty_min = pair_obj[c_pair_qty_min].get_double();
  result.m_qty_precision = pair_obj[c_pair_qty_precision].get_int64();

  buffer = pair_obj[c_pair_quote].get_string();
  result.m_quote = std::string(buffer.begin(), buffer.end());

  // TODO: improve error handling in the face of invalid status values
  buffer = pair_obj[c_pair_status].get_string();
  const auto status = std::string(buffer.begin(), buffer.end());
  const auto it = c_str_to_status.find(status);
  if (it != c_str_to_status.end()) {
    result.m_status = it->second;
  }

  buffer = pair_obj[c_pair_symbol].get_string();
  result.m_symbol = std::string(buffer.begin(), buffer.end());

  return result;
}

nlohmann::json pair_t::to_json() const {
  nlohmann::json result{
      {c_pair_base, m_base},
      {c_pair_cost_min, m_cost_min},
      {c_pair_cost_precision, m_cost_precision},
      {c_pair_has_index, m_has_index},
      {c_pair_price_increment, m_price_increment},
      {c_pair_price_precision, m_price_precision},
      {c_pair_qty_increment, m_qty_increment},
      {c_pair_qty_min, m_qty_min},
      {c_pair_qty_precision, m_qty_precision},
      {c_pair_quote, m_quote},
      {c_pair_symbol, m_symbol},
  };

  if (m_margin_initial) {
    result[c_pair_margin_initial] = *m_margin_initial;
  }

  result[c_pair_marginable] = m_marginable;

  if (m_position_limit_long) {
    result[c_pair_position_limit_long] = *m_position_limit_long;
  }

  if (m_position_limit_short) {
    result[c_pair_position_limit_short] = *m_position_limit_short;
  }

  // TODO: improve error handling in the face of invalid status values
  const auto it = c_status_to_str.find(m_status);
  if (it != c_status_to_str.end()) {
    result[c_pair_status] = it->second;
  }

  return result;
}

} // namespace response
} // namespace krakpot
