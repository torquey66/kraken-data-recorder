#include "refdata.hpp"

namespace krakpot {
namespace model {

void refdata_t::accept(const response::instrument_t& instrument) {
  for (const auto& asset : instrument.assets()) {
    accept(asset);
  }
  for (const auto& pair : instrument.pairs()) {
    accept(pair);
  }
}

void refdata_t::accept(const response::asset_t& asset) {
  m_assets[asset.id()] = asset;
}

void refdata_t::accept(const response::pair_t& pair) {
  m_pairs[pair.symbol()] = pair;
}

std::optional<refdata_t::pair_ref_t> refdata_t::pair(
    const std::string& symbol) const {
  std::optional<pair_ref_t> result;
  const auto it = m_pairs.find(symbol);
  if (it != m_pairs.end()) {
    const response::pair_t& pair{it->second};
    return std::make_optional(std::cref(pair));
  }
  return result;
}

std::optional<refdata_t::pair_precision_t> refdata_t::pair_precision(
    const std::string& symbol) const {
  std::optional<pair_precision_t> result;
  const auto it = m_pairs.find(symbol);
  if (it != m_pairs.end()) {
    const response::pair_t& pair{it->second};
    const pair_precision_t precision{pair.price_precision(),
                                     pair.qty_precision()};
    result = std::make_optional(precision);
  }
  return result;
}

}  // namespace model
}  // namespace krakpot
