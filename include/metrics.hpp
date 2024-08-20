#pragma once

#include "types.hpp"

#include <boost/json.hpp>

#include <cstddef>

namespace kdr {

struct metrics_t final {
  void accept(msg_t);

  boost::json::object to_json_obj() const;

  std::string str() const { return boost::json::serialize(to_json_obj()); }

private:
  const timestamp_t m_stm = timestamp_t::now();
  size_t m_num_msgs = 0;
  size_t m_num_bytes = 0;
};

} // namespace kdr
