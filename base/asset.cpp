/** Copyright (C) 2024 John C. Finley - All rights reserved */

#include "asset.hpp"

namespace krakpot {
namespace response {

const std::unordered_map<std::string, asset_t::status_t>
    asset_t::c_str_to_status = {
        {c_asset_status_depositonly, e_depositonly},
        {c_asset_status_disabled, e_disabled},
        {c_asset_status_enabled, e_enabled},
        {c_asset_status_fundingtemporarilydisabled, e_fundingtemporarilydisabled},
        {c_asset_status_withdrawalonly, e_withdrawalonly},
        {c_asset_status_workinprogress, e_workinprogress},
};

const std::unordered_map<asset_t::status_t, std::string>
    asset_t::c_status_to_str = {
        {e_invalid, c_asset_status_invalid},
        {e_depositonly, c_asset_status_depositonly},
        {e_disabled, c_asset_status_disabled},
        {e_enabled, c_asset_status_enabled},
        {e_fundingtemporarilydisabled, c_asset_status_fundingtemporarilydisabled},
        {e_withdrawalonly, c_asset_status_withdrawalonly},
        {e_workinprogress, c_asset_status_workinprogress},
};

asset_t asset_t::from_json(simdjson::ondemand::object &asset_obj) {
  auto result = asset_t{};
  auto buffer = std::string_view{};
  simdjson::ondemand::value optional_val{};

  result.m_borrowable = asset_obj[c_asset_borrowable].get_bool();
  result.m_collateral_value = asset_obj[c_asset_collateral_value].get_double();

  buffer = asset_obj[c_asset_id].get_string();
  result.m_id = std::string{buffer.begin(), buffer.end()};

  if (asset_obj[c_asset_margin_rate].get(optional_val) == simdjson::SUCCESS) {
    result.m_margin_rate = optional_val.get_double();
  }

  result.m_precision = asset_obj[c_asset_precision].get_int64();
  result.m_precision_display = asset_obj[c_asset_precision_display].get_int64();

  buffer = asset_obj[c_asset_status].get_string();
  const auto status = std::string(buffer.begin(), buffer.end());
  const auto it = c_str_to_status.find(status);
  if (it != c_str_to_status.end()) {
    result.m_status = it->second;
  }

  return result;
}

boost::json::object asset_t::to_json_obj() const {
  boost::json::object result = {
      {c_asset_borrowable, m_borrowable},
      {c_asset_collateral_value, m_collateral_value},
      {c_asset_id, m_id},
      {c_asset_precision, m_precision},
      {c_asset_precision_display, m_precision_display},
  };

  if (m_margin_rate) {
    result[c_asset_margin_rate] = *m_margin_rate;
  }

  const auto it = c_status_to_str.find(m_status);
  if (it != c_status_to_str.end()) {
    result[c_asset_status] = it->second;
  }
  return result;
}

} // namespace response
} // namespace krakpot
