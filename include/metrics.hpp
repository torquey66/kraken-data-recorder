#pragma once

#include "types.hpp"

#include <boost/json.hpp>

#include <cstddef>
#include <string_view>

namespace kdr {

struct metrics_t final {
  // clang-format off
  static constexpr std::string_view c_book_last_consumed       = "book_last_consumed";
  static constexpr std::string_view c_book_last_process_micros = "book_last_process_micros";
  static constexpr std::string_view c_book_max_queue_depth     = "book_max_queue_depth";
  static constexpr std::string_view c_book_queue_depth         = "book_queue_depth";
  static constexpr std::string_view c_num_bytes                = "num_bytes";
  static constexpr std::string_view c_num_heartbeats           = "num_heartbeats";
  static constexpr std::string_view c_num_msgs                 = "num_msgs";
  static constexpr std::string_view c_num_pings                = "num_pings";
  static constexpr std::string_view c_num_pongs                = "num_pongs";
  // clang-format on

  void accept(msg_t);

  void heartbeat() { ++m_num_heartbeats; }
  void ping() { ++m_num_pings; }
  void pong() { ++m_num_pongs; }

  void set_book_last_consumed(size_t consumed) {
    m_book_last_consumed = consumed;
  }
  void set_book_last_process_micros(size_t last_process_micros) {
    m_book_last_process_micros = last_process_micros;
  }
  void set_book_queue_depth(size_t depth) {
    m_book_queue_depth = depth;
    m_book_max_queue_depth =
        std::max(m_book_max_queue_depth, m_book_queue_depth);
  }

  boost::json::object to_json_obj() const;

  std::string str() const { return boost::json::serialize(to_json_obj()); }

private:
  const timestamp_t m_stm = timestamp_t::now();
  size_t m_book_last_consumed = 0;
  size_t m_book_last_process_micros = 0;
  size_t m_book_queue_depth = 0;
  size_t m_book_max_queue_depth = 0;
  size_t m_num_bytes = 0;
  size_t m_num_heartbeats = 0;
  size_t m_num_msgs = 0;
  size_t m_num_pings = 0;
  size_t m_num_pongs = 0;
};

} // namespace kdr
