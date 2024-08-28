#include "metrics.hpp"

#include "constants.hpp"

namespace kdr {

void metrics_t::accept(msg_t msg) {
  ++m_num_msgs;
  m_num_bytes += msg.size();
}

boost::json::object metrics_t::to_json_obj() const {
  const boost::json::object result = {
      {c_num_msgs, m_num_msgs},
      {c_num_bytes, m_num_bytes},
      {c_book_queue_depth, m_book_queue_depth},
      {c_book_max_queue_depth, m_book_max_queue_depth},
      {c_book_last_consumed, m_book_last_consumed},
      {c_book_last_process_micros, m_book_last_process_micros},
      {c_num_heartbeats, m_num_heartbeats},
      {c_num_pings, m_num_pings},
      {c_num_pongs, m_num_pongs},
  };
  return result;
}

} // namespace kdr
