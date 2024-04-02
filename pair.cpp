#include "pair.hpp"

namespace krakpot {
namespace response {

const std::unordered_map<std::string, pair_t::status_t>
    pair_t::c_str_to_status = {
        {"cancel_only", e_cancel_only},
        {"delisted", e_delisted},
        {"limit_only", e_limit_only},
        {"maintenance", e_maintenance},
        {"online", e_online},
        {"post_only", e_post_only},
        {"reduce_only", e_reduce_only},
        {"work_in_progress", e_work_in_progress},
};

const std::unordered_map<pair_t::status_t, std::string>
    pair_t::c_status_to_str = {
        {e_invalid, "invalid"},
        {e_cancel_only, "cancel_only"},
        {e_delisted, "delisted"},
        {e_limit_only, "limit_only"},
        {e_maintenance, "maintenance"},
        {e_online, "online"},
        {e_post_only, "post_only"},
        {e_reduce_only, "reduce_only"},
        {e_work_in_progress, "work_in_progress"},
};

pair_t pair_t::from_json(simdjson::ondemand::object &pair_obj) {
  auto result = pair_t{};
  auto buffer = std::string_view{};
  simdjson::ondemand::value optional_val{};

  buffer = pair_obj["base"].get_string();
  result.m_base = std::string{buffer.begin(), buffer.end()};

  result.m_cost_min = pair_obj["cost_min"].get_double();
  result.m_cost_precision = pair_obj["cost_precision"].get_int64();
  result.m_has_index = pair_obj["has_index"].get_bool();

  if (pair_obj["margin_initial"].get(optional_val) == simdjson::SUCCESS) {
    result.m_margin_initial = optional_val.get_double();
  }

  result.m_marginable = pair_obj["marginable"].get_bool();

  if (pair_obj["position_limit_long"].get(optional_val) == simdjson::SUCCESS) {
    result.m_position_limit_long = optional_val.get_int64();
  }
  if (pair_obj["position_limit_short"].get(optional_val) == simdjson::SUCCESS) {
    result.m_position_limit_short = optional_val.get_int64();
  }

  result.m_price_increment = pair_obj["price_increment"].get_double();
  result.m_price_precision = pair_obj["price_precision"].get_int64();
  result.m_qty_increment = pair_obj["qty_increment"].get_double();
  result.m_qty_min = pair_obj["qty_min"].get_double();
  result.m_qty_precision = pair_obj["qty_precision"].get_int64();

  buffer = pair_obj["quote"].get_string();
  result.m_quote = std::string(buffer.begin(), buffer.end());

  // TODO: improve error handling in the face of invalid status values
  buffer = pair_obj["status"].get_string();
  const auto status = std::string(buffer.begin(), buffer.end());
  const auto it = c_str_to_status.find(status);
  if (it != c_str_to_status.end()) {
    result.m_status = it->second;
  }

  buffer = pair_obj["symbol"].get_string();
  result.m_symbol = std::string(buffer.begin(), buffer.end());

  return result;
}

nlohmann::json pair_t::to_json() const {
  nlohmann::json result{
      {"base", m_base},
      {"cost_min", m_cost_min},
      {"cost_precision", m_cost_precision},
      {"has_index", m_has_index},
      {"price_increment", m_price_increment},
      {"price_precision", m_price_precision},
      {"qty_increment", m_qty_increment},
      {"qty_min", m_qty_min},
      {"qty_precision", m_qty_precision},
      {"quote", m_quote},
      {"symbol", m_symbol},
  };

  if (m_margin_initial) {
    result["margin_initial"] = *m_margin_initial;
  }

  result["marginable"] = m_marginable;

  if (m_position_limit_long) {
    result["position_limit_long"] = *m_position_limit_long;
  }

  if (m_position_limit_short) {
    result["position_limit_short"] = *m_position_limit_short;
  }

  // TODO: improve error handling in the face of invalid status values
  const auto it = c_status_to_str.find(m_status);
  if (it != c_status_to_str.end()) {
    result["status"] = it->second;
  }

  return result;
}

} // namespace response
} // namespace krakpot
