#include "requests.hpp"

#include <nlohmann/json.hpp>

namespace krakpot {

std::string subscribe_instrument_t::to_json() const {
  const nlohmann::json result = {
      {"method", "subscribe"},
      {"params", {{"channel", "instrument"}, {"snapshot", m_snapshot}}},
      {"req_id", m_req_id}};
  return result.dump();
}

std::string subscribe_book_t::to_json() const {
  const nlohmann::json result = {{"method", "subscribe"},
                                 {"params",
                                  {{"channel", "book"},
                                   {"depth", m_depth},
                                   {"snapshot", m_snapshot},
                                   {"symbol", m_symbols}}},
                                 {"req_id", m_req_id}};
  return result.dump();
}

} // namespace krakpot
