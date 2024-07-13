/* Copyright (C) 2024 John C. Finley - All rights reserved */
#pragma once

#include "types.hpp"

#include <simdjson.h>  // !@#
#include <boost/json.hpp>

#include <optional>
#include <string>
#include <unordered_map>

namespace krakpot {
namespace response {

/**
 * Internal representation of the "Response Data-Assets" field desribed here:
 *
 *     https://docs.kraken.com/websockets-v2/#instrument
 */
struct asset_t final {
  // clang-format off
  enum status_t : int8_t {
    e_invalid                    = -1,
    e_depositonly                = 0,
    e_disabled                   = 1,
    e_enabled                    = 2,
    e_fundingtemporarilydisabled = 3,
    e_withdrawalonly             = 4,
    e_workinprogress             = 5,
  };
  // clang-format on

  asset_t() {}
  asset_t(bool borrowable,
          double_t collateral_value,
          std::string id,
          std::optional<double_t> margin_rate,
          integer_t precision,
          integer_t precision_display,
          status_t status);

  bool operator==(const asset_t&) const;

  bool borrowable() const { return m_borrowable; }
  double_t collateral_value() const { return m_collateral_value; }
  std::string id() const { return m_id; }
  std::optional<double_t> margin_rate() const { return m_margin_rate; }
  integer_t precision() const { return m_precision; }
  integer_t precision_display() const { return m_precision_display; }
  status_t status() const { return m_status; }

  boost::json::object to_json_obj() const;
  std::string str() const { return boost::json::serialize(to_json_obj()); }

  static asset_t from_json_obj(const boost::json::object&);
  static asset_t from_json(simdjson::ondemand::object& asset_obj);  // !@#

 private:
  static const std::unordered_map<std::string, status_t> c_str_to_status;
  static const std::unordered_map<status_t, std::string> c_status_to_str;

  bool m_borrowable = false;
  double_t m_collateral_value = 0.;
  std::string m_id;
  std::optional<double_t> m_margin_rate;
  integer_t m_precision = 0;
  integer_t m_precision_display = 0;
  status_t m_status = e_invalid;
};

}  // namespace response
}  // namespace krakpot
