/* Copyright (C) 2024 John C. Finley - All rights reserved */

#include "metrics.hpp"

namespace krakpot {

void metrics_t::accept(msg_t msg) {
  ++m_num_msgs;
  m_num_bytes += msg.size();
}

nlohmann::json metrics_t::to_json() const {
  const nlohmann::json result = {
      {"num_msgs", m_num_msgs},
      {"num_bytes", m_num_bytes},
  };
  return result;
}

} // namespace krakpot
