#include <boost/interprocess/managed_shared_memory.hpp>

#include "depth.hpp"
#include "shmem_sink.hpp"
#include "types.hpp"

#include <array>

namespace kdr {
namespace shmem {

struct book_content_t final {
  static constexpr size_t c_max_depth = kdr::model::depth_1000;

  size_t num_quotes() const { return m_num_quotes; }
  const kdr::quote_t &quote(size_t idx) { return m_quotes.at(idx); }

private:
  size_t m_num_quotes;
  std::array<kdr::quote_t, c_max_depth> m_quotes;
};

struct book_obj_t final {
  book_obj_t(std::string symbol) : m_obj_name{"kdr::book_obj_t::" + symbol} {
    boost::interprocess::shared_memory_object::remove(m_obj_name.c_str());
  }

  ~book_obj_t() {
    boost::interprocess::shared_memory_object::remove(m_obj_name.c_str());
  }

  const std::string &obj_name() const { return m_obj_name; }

private:
  const std::string m_obj_name;
};

struct book_segment_t final {
  book_segment_t(std::string symbol)
      : m_book_obj{symbol},
        m_segment{boost::interprocess::create_only,
                  m_book_obj.obj_name().c_str(), sizeof(book_content_t)},
        m_name{"kdr::book_content_t::" + symbol},
        m_content{m_segment.construct<book_content_t>(m_name.c_str())()} {}

  ~book_segment_t() {
    if (m_content) {
      m_segment.destroy<book_content_t>(m_name.c_str());
    }
  }

private:
  book_obj_t m_book_obj;
  boost::interprocess::managed_shared_memory m_segment;
  std::string m_name;
  book_content_t *m_content = nullptr;
};

void shmem_sink_t::accept(const response::instrument_t &response) {
  for (const model::pair_t &pair : response.pairs()) {
    const auto it = m_book_segments.find(pair.symbol());
    if (it == m_book_segments.end()) {
      book_segment_ptr book_segment =
          std::make_unique<book_segment_t>(pair.symbol());
      m_book_segments.insert(
          std::make_pair(pair.symbol(), std::move(book_segment)));
    }
  }
}

void shmem_sink_t::accept(const response::book_t & /*response*/) {}

} // namespace shmem
} // namespace kdr
