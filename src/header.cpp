#include "header.hpp"

#include <array>

namespace kdr {
namespace response {

static constexpr auto c_recv_tm_field = std::to_array("recv_tm");
static constexpr auto c_channel_field = std::to_array("channel");
static constexpr auto c_type_field = std::to_array("type");

const std::string_view header_t::c_recv_tm{c_recv_tm_field.data(),
                                           c_recv_tm_field.size() - 1};

const std::string_view header_t::c_channel{c_channel_field.data(),
                                           c_channel_field.size() - 1};

const std::string_view header_t::c_type{c_type_field.data(),
                                        c_type_field.size() - 1};

header_t::header_t(timestamp_t recv_tm, std::string channel, std::string type)
    : m_recv_tm(recv_tm), m_channel(channel), m_type(type) {}

boost::json::object header_t::to_json_obj() const {
  const boost::json::object result = {
      {c_recv_tm, recv_tm().str()}, {c_channel, channel()}, {c_type, type()}};
  return result;
}

}  // namespace response
}  // namespace kdr
