#pragma once

#include "pair.hpp"

#include "nlohmann/json.hpp"
#include <simdjson.h>

#include <string>
#include <vector>

namespace krakpot {
namespace response {

struct header_t final {

  header_t() = default;
  header_t(std::string channel, std::string type)
      : m_channel(channel), m_type(type) {}

  const std::string &channel() const { return m_channel; }
  const std::string &type() const { return m_type; }

private:
  std::string m_channel;
  std::string m_type;
};

struct instrument_t final {

  instrument_t() = default;

  static instrument_t from_json(simdjson::ondemand::document &);

  nlohmann::json to_json() const;
  std::string str() const { return to_json().dump(); }

private:
  header_t m_header;
  // !@# TODO: define asset_t
  // std::vector<asset_t> m_assets;
  std::vector<pair_t> m_pairs;
};

} // namespace response
} // namespace krakpot
