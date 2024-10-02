#pragma once

#include "book.hpp"
#include "instrument.hpp"

#include <memory>
#include <unordered_map>

namespace kdr {
namespace shmem {

struct book_segment_t;

struct shmem_sink_t final {

  void accept(const response::instrument_t &response);
  void accept(const response::book_t & /*response*/);

private:
  using book_segment_ptr = std::unique_ptr<book_segment_t>;
  std::unordered_map<std::string, book_segment_ptr> m_book_segments;
};

} // namespace shmem
} // namespace kdr
