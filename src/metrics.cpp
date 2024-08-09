#include "metrics.hpp"

#include "constants.hpp"

namespace kdr {

void metrics_t::accept(msg_t msg) {
  ++m_num_msgs;
  m_num_bytes += msg.size();
}

boost::json::object metrics_t::to_json_obj() const {
  const boost::json::object result = {
      {c_metrics_num_msgs, m_num_msgs},
      {c_metrics_num_bytes, m_num_bytes},
  };
  return result;
}

}  // namespace kdr
