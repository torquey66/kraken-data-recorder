#pragma once

#include "timestamp.hpp"

#include <boost/json.hpp>

#include <string>
#include <string_view>

namespace kdr {
namespace response {

struct header_t final {
  /** Field names */
  static const std::string_view c_recv_tm;
  static const std::string_view c_channel;
  static const std::string_view c_type;

  header_t() = default;
  header_t(timestamp_t recv_tm, std::string channel, std::string type);

  timestamp_t recv_tm() const { return m_recv_tm; }
  const std::string &channel() const { return m_channel; }
  const std::string &type() const { return m_type; }

  boost::json::object to_json_obj() const;
  std::string str() const { return boost::json::serialize(to_json_obj()); }

private:
  timestamp_t m_recv_tm;
  std::string m_channel;
  std::string m_type;
};

} // namespace response
} // namespace kdr
