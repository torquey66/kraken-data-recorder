#pragma once

#include "asset.hpp"
#include "header.hpp"
#include "pair.hpp"

#include <simdjson.h>
#include <boost/json.hpp>

#include <string>
#include <vector>

namespace kdr {
namespace response {

/**
 * See https://docs.kraken.com/websockets-v2/#instrument
 */
struct instrument_t final {
  instrument_t() = default;

  static instrument_t from_json(simdjson::ondemand::document&);

  const header_t& header() const { return m_header; }
  const std::vector<model::asset_t>& assets() const { return m_assets; }
  const std::vector<model::pair_t>& pairs() const { return m_pairs; }

  boost::json::object to_json_obj() const;
  std::string str() const { return boost::json::serialize(to_json_obj()); }

 private:
  header_t m_header;
  std::vector<model::asset_t> m_assets;
  std::vector<model::pair_t> m_pairs;
};

}  // namespace response
}  // namespace kdr
