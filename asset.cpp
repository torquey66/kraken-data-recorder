/** Copyright (C) 2024 John C. Finley - All rights reserved */

#include "asset.hpp"

namespace krakpot {
namespace response {

const std::unordered_map<std::string, asset_t::status_t>
    asset_t::c_str_to_status = {
        {"depositonly", e_depositonly},
        {"disabled", e_disabled},
        {"enabled", e_enabled},
        {"fundingtemporarilydisabled", e_fundingtemporarilydisabled},
        {"withdrawalonly", e_withdrawalonly},
        {"workinprogress", e_workinprogress},
};

const std::unordered_map<asset_t::status_t, std::string>
    asset_t::c_status_to_str = {
        {e_invalid, "invalid"},
        {e_depositonly, "depositonly"},
        {e_disabled, "disabled"},
        {e_enabled, "enabled"},
        {e_fundingtemporarilydisabled, "fundingtemporarilydisabled"},
        {e_withdrawalonly, "withdrawalonly"},
        {e_workinprogress, "workinprogress"},
};

asset_t asset_t::from_json(simdjson::ondemand::object &asset_obj) {
  auto result = asset_t{};
  auto buffer = std::string_view{};
  simdjson::ondemand::value optional_val{};

  result.m_borrowable = asset_obj["borrowable"].get_bool();
  result.m_collateral_value = asset_obj["collateral_value"].get_double();

  buffer = asset_obj["id"].get_string();
  result.m_id = std::string{buffer.begin(), buffer.end()};

  if (asset_obj["margin_rate"].get(optional_val) == simdjson::SUCCESS) {
    result.m_margin_rate = optional_val.get_double();
  }

  result.m_precision = asset_obj["precision"].get_int64();
  result.m_precision_display = asset_obj["precision_display"].get_int64();

  buffer = asset_obj["status"].get_string();
  const auto status = std::string(buffer.begin(), buffer.end());
  const auto it = c_str_to_status.find(status);
  if (it != c_str_to_status.end()) {
    result.m_status = it->second;
  }

  return result;
}

nlohmann::json asset_t::to_json() const {
  nlohmann::json result = {
      {"borrowable", m_borrowable},
      {"collateral_value", m_collateral_value},
      {"id", m_id},
      {"precision", m_precision},
      {"precision_display", m_precision_display},
  };

  if (m_margin_rate) {
    result["margin_rate"] = *m_margin_rate;
  }

  const auto it = c_status_to_str.find(m_status);
  if (it != c_status_to_str.end()) {
    result["status"] = it->second;
  }

  return result;
}

} // namespace response
} // namespace krakpot
