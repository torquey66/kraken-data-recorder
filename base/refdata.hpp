/* Copyright (C) 2024 John C. Finley - All rights reserved */
#pragma once

#include "responses.hpp"

#include <optional>
#include <unordered_map>

namespace krakpot {

struct refdata_t final {
  void accept(const response::instrument_t&);

  std::optional<response::asset_t&> find_asset(const std::string& id);
  std::optional<response::pair_t&> find_pair(const std::string& symbol);

 private:
  std::unordered_map<std::string, response::asset_t> m_assets;
  std::unordered_map<std::string, response::pair_t> m_pairs;
};

}  // namespace krakpot
