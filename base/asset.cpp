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

asset_t::asset_t(bool borrowable,
                 double_t collateral_value,
                 std::string id,
                 std::optional<double_t> margin_rate,
                 integer_t precision,
                 integer_t precision_display,
                 status_t status)
    : m_borrowable{borrowable},
      m_collateral_value{collateral_value},
      m_id{id},
      m_margin_rate{margin_rate},
      m_precision{precision},
      m_precision_display{precision_display},
      m_status{status} {}

asset_t asset_t::from_json(simdjson::ondemand::object& asset_obj) {
  const bool borrowable{asset_obj[c_asset_borrowable].get_bool()};
  const double collateral_value{
      asset_obj[c_asset_collateral_value].get_double()};

  const std::string_view id_view{asset_obj[c_asset_id].get_string()};
  const std::string id{id_view.data(), id_view.size()};

  std::optional<double_t> margin_rate;
  simdjson::ondemand::value optional_val{};
  if (asset_obj[c_asset_margin_rate].get(optional_val) == simdjson::SUCCESS) {
    margin_rate = optional_val.get_double();
  }

  const integer_t precision{asset_obj[c_asset_precision].get_int64()};
  const integer_t precision_display{
      asset_obj[c_asset_precision_display].get_int64()};

  const std::string_view status_view{asset_obj[c_asset_status].get_string()};
  const std::string status_str{status_view.data(), status_view.size()};
  const auto it = c_str_to_status.find(status_str);
  if (it == c_str_to_status.end()) {
    throw std::runtime_error("invalid status: '" + status_str + "'");
  }
  const status_t status{it->second};

  return asset_t{borrowable, collateral_value,  id,    margin_rate,
                 precision,  precision_display, status};
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

}  // namespace response
}  // namespace krakpot
