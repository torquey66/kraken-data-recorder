#include "responses.hpp"

namespace krakpot {
namespace response {

instrument_t instrument_t::from_json(simdjson::ondemand::document &response) {
  auto result = instrument_t{};
  auto buffer = std::string_view{};

  buffer = response["channel"].get_string();
  const auto channel = std::string{buffer.begin(), buffer.end()};
  buffer = response["type"].get_string();
  const auto type = std::string{buffer.begin(), buffer.end()};
  result.m_header = header_t{channel, type};

  // !@# TODO: populate assets here...

  for (simdjson::fallback::ondemand::object obj : response["data"]["pairs"]) {
    const auto pair = pair_t::from_json(obj);
    result.m_pairs.push_back(pair);
  }

  return result;
}

nlohmann::json instrument_t::to_json() const {
  auto pairs = nlohmann::json{};
  for (const auto &pair : m_pairs) {
    pairs.push_back(pair.to_json());
  }
  auto data = nlohmann::json{};
  data["pairs"] = pairs;
  const nlohmann::json result = {{"channel", m_header.channel()},
                                 {"data", data},
                                 {"type", m_header.type()}};

  return result;
}

} // namespace response
} // namespace krakpot
