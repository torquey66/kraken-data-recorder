#pragma once

#include "types.hpp"

#include "../generated/depth.hpp"  // !@# temporary

#include <boost/json.hpp>

#include <string>
#include <vector>

/**
 * These are the various messages we currently send to the
 * venue. Ideally, this code would be generated from an OpenAPI spec
 * or the like, but that exercise is currently beyond scope. Also,
 * according to ChatGPT, Kraken has not released an OpenAPI spec for
 * its websocket interface.
 */
namespace kdr {
namespace request {

/**
 * See https://docs.kraken.com/websockets-v2/#ping
 */
struct ping_t final {
  ping_t(req_id_t req_id) : m_req_id(req_id) {}

  boost::json::object to_json_obj() const;
  std::string str() const { return boost::json::serialize(to_json_obj()); }

 private:
  req_id_t m_req_id;
};

/**
 * See https://docs.kraken.com/websockets-v2/#instrument
 */
struct subscribe_instrument_t final {
  subscribe_instrument_t(req_id_t req_id, bool snapshot = true)
      : m_req_id(req_id), m_snapshot(snapshot) {}

  boost::json::object to_json_obj() const;
  std::string str() const { return boost::json::serialize(to_json_obj()); }

 private:
  req_id_t m_req_id;
  bool m_snapshot;
};

/**
 * See https://docs.kraken.com/websockets-v2/#book
 */
struct subscribe_book_t final {
  subscribe_book_t(req_id_t req_id,
                   model::depth_t depth,
                   bool snapshot,
                   const std::vector<std::string>& symbols)
      : m_req_id{req_id},
        m_depth{depth},
        m_snapshot{snapshot},
        m_symbols{symbols} {}

  boost::json::object to_json_obj() const;
  std::string str() const { return boost::json::serialize(to_json_obj()); }

 private:
  req_id_t m_req_id;
  model::depth_t m_depth;
  bool m_snapshot;
  std::vector<std::string> m_symbols;
};

/**
 * See https://docs.kraken.com/websockets-v2/#trade
 */
struct subscribe_trade_t final {
  subscribe_trade_t(req_id_t req_id,
                    bool snapshot,
                    const std::vector<std::string>& symbols)
      : m_req_id(req_id), m_snapshot(snapshot), m_symbols(symbols) {}

  boost::json::object to_json_obj() const;
  std::string str() const { return boost::json::serialize(to_json_obj()); }

 private:
  req_id_t m_req_id;
  bool m_snapshot;
  std::vector<std::string> m_symbols;
};

}  // namespace request
}  // namespace kdr
