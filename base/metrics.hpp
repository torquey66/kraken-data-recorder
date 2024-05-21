/* Copyright (C) 2024 John C. Finley - All rights reserved */

#pragma once

#include "types.hpp"

#include <nlohmann/json.hpp>

#include <cstddef>

namespace krakpot {

struct metrics_t final {

  void accept(msg_t);

  nlohmann::json to_json() const;
  std::string str() const { return to_json().dump(); }

private:
  const timestamp_t m_stm = timestamp_t::now();
  size_t m_num_msgs = 0;
  size_t m_num_bytes = 0;
};

} // namespace krakpot
