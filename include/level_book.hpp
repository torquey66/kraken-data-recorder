#pragma once

#include "sides.hpp"

#include <unordered_map>

namespace kdr {
namespace model {

struct level_book_t final {
  using symbol_t = std::string;

  const sides_t &sides(symbol_t) const;

  level_book_t(depth_t book_depth) : m_book_depth{book_depth} {};

  void accept(const model::pair_t &);
  void accept(const response::book_t &);

  uint64_t crc32(symbol_t symbol) const;

  std::string str(std::string) const;

private:
  depth_t m_book_depth;
  std::unordered_map<symbol_t, sides_t> m_sides;
};

} // namespace model
} // namespace kdr
