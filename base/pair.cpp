/* Copyright (C) 2024 John C. Finley - All rights reserved */

#include "pair.hpp"

#include "extract.hpp"

#include <boost/log/trivial.hpp>

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

pair_t::pair_t(std::string base,
               decimal_t cost_min,
               integer_t cost_precision,
               bool has_index,
               std::optional<double_t> margin_initial,
               bool marginable,
               std::optional<integer_t> position_limit_long,
               std::optional<integer_t> position_limit_short,
               decimal_t price_increment,
               integer_t price_precision,
               decimal_t qty_increment,
               decimal_t qty_min,
               integer_t qty_precision,
               std::string quote,
               status_t status,
               std::string symbol)
    : m_base{base},
      m_cost_min{cost_min},
      m_cost_precision{cost_precision},
      m_has_index{has_index},
      m_margin_initial{margin_initial},
      m_marginable{marginable},
      m_position_limit_long{position_limit_long},
      m_position_limit_short{position_limit_short},
      m_price_increment{price_increment},
      m_price_precision{price_precision},
      m_qty_increment{qty_increment},
      m_qty_min{qty_min},
      m_qty_precision{qty_precision},
      m_quote{quote},
      m_status{status},
      m_symbol{symbol} {}

pair_t pair_t::from_json_obj(const boost::json::object& pair_obj) {
  auto result = pair_t{};

  try {
    result.m_base = pair_obj.at(c_pair_base).as_string();

    result.m_cost_min = extract_decimal(pair_obj.at(c_pair_cost_min));
    result.m_cost_precision = pair_obj.at(c_pair_cost_precision).as_int64();
    result.m_has_index = pair_obj.at(c_pair_has_index).as_bool();

    if (pair_obj.contains(c_pair_margin_initial)) {
      result.m_margin_initial =
          extract_double(pair_obj.at(c_pair_margin_initial));
    }

    result.m_marginable = pair_obj.at(c_pair_marginable).as_bool();

    if (pair_obj.contains(c_pair_position_limit_long)) {
      result.m_position_limit_long =
          pair_obj.at(c_pair_position_limit_long).as_int64();
    }

    if (pair_obj.contains(c_pair_position_limit_short)) {
      result.m_position_limit_short =
          pair_obj.at(c_pair_position_limit_long).as_int64();
    }

    result.m_price_increment =
        extract_decimal(pair_obj.at(c_pair_price_increment));
    result.m_price_precision = pair_obj.at(c_pair_price_precision).as_int64();
    result.m_qty_increment = extract_decimal(pair_obj.at(c_pair_qty_increment));
    result.m_qty_min = extract_decimal(pair_obj.at(c_pair_qty_min));
    result.m_qty_precision = pair_obj.at(c_pair_qty_precision).as_int64();

    result.m_quote = pair_obj.at(c_pair_quote).as_string();

    const auto status_str = std::string{pair_obj.at(c_pair_status).as_string()};
    const auto it = c_str_to_status.find(status_str);
    if (it != c_str_to_status.end()) {
      result.m_status = it->second;
    }

    result.m_symbol = pair_obj.at(c_pair_symbol).as_string();
  } catch (const std::exception& ex) {
    BOOST_LOG_TRIVIAL(error) << __FUNCTION__ << " what: " << ex.what()
                             << " msg: " << boost::json::serialize(pair_obj);
    throw ex;
  }

  return result;
}

boost::json::object pair_t::to_json_obj() const {
  boost::json::object result{
      {c_pair_base, m_base},
      {c_pair_cost_min, m_cost_min.str(m_cost_precision)},
      {c_pair_cost_precision, m_cost_precision},
      {c_pair_has_index, m_has_index},
      {c_pair_price_increment, m_price_increment.str(m_price_precision)},
      {c_pair_price_precision, m_price_precision},
      {c_pair_qty_increment, m_qty_increment.str(m_qty_precision)},
      {c_pair_qty_min, m_qty_min.str(m_qty_precision)},
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

}  // namespace response
}  // namespace krakpot
