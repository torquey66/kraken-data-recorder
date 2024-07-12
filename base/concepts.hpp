#pragma once

#include "types.hpp"

#include <boost/json.hpp>

namespace krakpot {

template <typename T>
concept json_serializable = requires(T v) {
  { v.to_json_obj() } -> std::convertible_to<boost::json::object>;
  { v.str() } -> std::convertible_to<std::string>;
};

template <typename T>
concept is_request = requires(T v) {
  { v.req_id() } -> std::convertible_to<req_id_t>;
};

}  // namespace krakpot
