/* Copyright (C) 2024 John C. Finley - All rights reserved */
#pragma once

#include "responses.hpp"
#include "types.hpp"

#include <functional>
#include <optional>
#include <unordered_map>

namespace krakpot {
namespace model {

struct refdata_t final {
  struct pair_precision_t final {
    precision_t price_precision = 0;
    precision_t qty_precision = 0;
  };

  void accept(const response::instrument_t&);

  using pair_ref_t = std::reference_wrapper<const response::pair_t>;
  std::optional<pair_ref_t> pair(const std::string&) const;

  std::optional<pair_precision_t> pair_precision(const std::string&) const;

 private:
  std::unordered_map<std::string, response::asset_t> m_assets;
  std::unordered_map<std::string, response::pair_t> m_pairs;
};

}  // namespace model
}  // namespace krakpot
