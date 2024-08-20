#include "refdata.hpp"

namespace kdr {
namespace model {

void refdata_t::accept(const response::instrument_t &instrument) {
  for (const auto &asset : instrument.assets()) {
    m_assets[asset.id()] = asset;
  }
  for (const auto &pair : instrument.pairs()) {
    m_pairs[pair.symbol()] = pair;
  }
}

std::optional<refdata_t::pair_precision_t>
refdata_t::pair_precision(const std::string &symbol) const {
  std::optional<pair_precision_t> result;
  const auto it = m_pairs.find(symbol);
  if (it != m_pairs.end()) {
    const model::pair_t &pair{it->second};
    const pair_precision_t precision{pair.price_precision(),
                                     pair.qty_precision()};
    result = std::make_optional(precision);
  }
  return result;
}

} // namespace model
} // namespace kdr
