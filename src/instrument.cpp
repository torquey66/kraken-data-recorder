#include "instrument.hpp"

#include <algorithm>

namespace kdr {
namespace response {

instrument_t instrument_t::from_json(simdjson::ondemand::document& response) {
  auto result = instrument_t{};
  auto buffer = std::string_view{};

  buffer = response[header_t::c_channel].get_string();
  const auto channel = std::string{buffer.begin(), buffer.end()};
  buffer = response[header_t::c_type].get_string();
  const auto type = std::string{buffer.begin(), buffer.end()};
  result.m_header = header_t{timestamp_t::now(), channel, type};

  for (simdjson::fallback::ondemand::object obj :
       response[c_response_data][c_instrument_assets]) {
    const auto asset = model::asset_t::from_json(obj);
    result.m_assets.push_back(asset);
  }

  for (simdjson::fallback::ondemand::object obj :
       response[c_response_data][c_instrument_pairs]) {
    const auto pair = model::pair_t::from_json(obj);
    result.m_pairs.push_back(pair);
  }

  return result;
}

boost::json::object instrument_t::to_json_obj() const {
  auto assets = boost::json::array{};
  std::transform(
      m_assets.begin(), m_assets.end(), std::back_inserter(assets),
      [](const model::asset_t& asset) { return asset.to_json_obj(); });

  auto pairs = boost::json::array{};
  std::transform(m_pairs.begin(), m_pairs.end(), std::back_inserter(pairs),
                 [](const model::pair_t& pair) { return pair.to_json_obj(); });

  auto data = boost::json::object{};
  data[c_instrument_pairs] = pairs;
  data[c_instrument_assets] = assets;

  const boost::json::object result = {{header_t::c_channel, m_header.channel()},
                                      {c_response_data, data},
                                      {header_t::c_type, m_header.type()}};

  return result;
}

}  // namespace response
}  // namespace kdr
