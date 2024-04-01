#pragma once

#include "nlohmann/json.hpp"

#include <string>
#include <vector>

namespace krakpot {
namespace request {

using req_id_t = int64_t;

/**
 * See https://docs.kraken.com/websockets-v2/#instrument
 */
struct subscribe_instrument_t final {

  subscribe_instrument_t(req_id_t req_id, bool snapshot = true)
      : m_req_id(req_id), m_snapshot(snapshot) {}

  nlohmann::json to_json() const;
  std::string str() const { return to_json().dump(); }

private:
  req_id_t m_req_id;
  bool m_snapshot;
};

/**
 * See https://docs.kraken.com/websockets-v2/#book
 */
struct subscribe_book_t final {
  /**
   * Allowed values for book subscriptions.
   */
  enum depth_t : int64_t {
    e_invalid = -1,
    e_10 = 10,
    e_25 = 25,
    e_100 = 100,
    e_500 = 500,
    e_1000 = 1000,
  };

  subscribe_book_t(req_id_t req_id, depth_t depth, bool snapshot,
                   const std::vector<std::string> &symbols)
      : m_req_id(req_id), m_depth(depth), m_snapshot(snapshot),
        m_symbols(symbols) {}

  nlohmann::json to_json() const;
  std::string str() const { return to_json().dump(); }

private:
  req_id_t m_req_id;
  depth_t m_depth;
  bool m_snapshot;
  std::vector<std::string> m_symbols;
};

} // namespace request
} // namespace krakpot
