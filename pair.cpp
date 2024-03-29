#include "pair.hpp"

#include <simdjson.h>

namespace krakpot {

const std::unordered_map<std::string, pair_t::status_t>
    pair_t::c_str_to_status = {
        {"cancel_only", e_cancel_only},
        {"delisted", e_delisted},
        {"limit_only", e_limit_only},
        {"maintenance", e_maintenance},
        {"online", e_online},
        {"post_only", e_post_only},
        {"reduce_only", e_reduce_only},
        {"work_in_progress", e_work_in_progress},
};

const std::unordered_map<pair_t::status_t, std::string>
    pair_t::c_status_to_str = {
        {e_cancel_only, "cancel_only"},
        {e_delisted, "delisted"},
        {e_limit_only, "limit_only"},
        {e_maintenance, "maintenance"},
        {e_online, "online"},
        {e_post_only, "post_only"},
        {e_reduce_only, "reduce_only"},
        {e_work_in_progress, "work_in_progress"},
};

pair_t from_json(std::string_view jv) {
  auto result = pair_t{};
  return result;
}

} // namespace krakpot
