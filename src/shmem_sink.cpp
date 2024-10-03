#include <boost/interprocess/managed_shared_memory.hpp>
#include <boost/interprocess/sync/named_mutex.hpp>
#include <boost/interprocess/sync/scoped_lock.hpp>

#include "depth.hpp"
#include "shmem_sink.hpp"
#include "types.hpp"

#include <boost/log/trivial.hpp>

#include <array>

namespace kdr {
namespace shmem {

/**
 * Shared memory representation of book state.
 */
struct book_content_t final {
  static constexpr size_t c_max_depth = kdr::model::depth_1000;

  size_t num_bids() const { return m_num_bids; }
  const kdr::quote_t &bid(size_t idx) const { return m_bids.at(idx); }

  size_t num_asks() const { return m_num_asks; }
  const kdr::quote_t &ask(size_t idx) const { return m_asks.at(idx); }

  void accept(const model::sides_t &sides) {
    if (sides.book_depth() < 0) {
      const auto message = std::string(__FUNCTION__) + " invalid book_depth: " +
                           std::to_string(sides.book_depth());
      throw std::runtime_error(message);
    }
    if (static_cast<size_t>(sides.book_depth()) > c_max_depth) {
      const auto message =
          std::string(__FUNCTION__) +
          " book_depth: " + std::to_string(sides.book_depth()) +
          " exceeds c_max_depth: " + std::to_string(c_max_depth);
      throw std::runtime_error(message);
    }
    update_side(m_bids, m_num_bids, sides.bids());
    update_side(m_asks, m_num_asks, sides.asks());
  }

private:
  template <typename S>
  static void update_side(std::array<kdr::quote_t, c_max_depth> &out_quotes,
                          size_t &out_num_quotes, const S &side) {
    out_num_quotes = side.size();
    size_t idx = 0;
    for (const auto &pq : side) {
      out_quotes[idx++] = pq;
    }
  }

  size_t m_num_bids = 0;
  size_t m_num_asks = 0;
  std::array<kdr::quote_t, c_max_depth> m_bids;
  std::array<kdr::quote_t, c_max_depth> m_asks;
};

/**
 * RAII wrapper of named shared memory object.
 */
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

/**
 * RAII wrapper of named mutex.
 */
struct book_mutex_t final {
  book_mutex_t(std::string symbol)
      : m_mutex_name{"kdr::book_mutex_t::" + symbol} {
    boost::interprocess::named_mutex::remove(m_mutex_name.c_str());
  }

  ~book_mutex_t() {
    boost::interprocess::named_mutex::remove(m_mutex_name.c_str());
  }

  const std::string &mutex_name() const { return m_mutex_name; }

private:
  const std::string m_mutex_name;
};

/**
 * RAII wrapper of shared memory segment, etc.
 */
struct book_segment_t final {
  book_segment_t(std::string symbol)
      : m_book_obj{symbol},
        m_segment{boost::interprocess::create_only,
                  m_book_obj.obj_name().c_str(), sizeof(book_content_t)},
        m_name{"kdr::book_content_t::" + symbol},
        m_content{m_segment.construct<book_content_t>(m_name.c_str())()},
        m_book_mutex(symbol), m_mutex{boost::interprocess::create_only,
                                      m_book_mutex.mutex_name().c_str()} {}

  ~book_segment_t() {
    if (m_content) {
      m_segment.destroy<book_content_t>(m_name.c_str());
    }
  }

  void accept(const model::sides_t &sides) {
    if (!m_content) {
      const auto message =
          std::string(__FUNCTION__) + " unexpected null_ptr m_content";
      throw std::runtime_error(message);
    }

    boost::interprocess::scoped_lock<boost::interprocess::named_mutex> lock;
    m_content->accept(sides);
  }

private:
  book_obj_t m_book_obj;
  boost::interprocess::managed_shared_memory m_segment;
  std::string m_name;
  book_content_t *m_content = nullptr;
  book_mutex_t m_book_mutex;
  boost::interprocess::named_mutex m_mutex;
};

/**
 * Create shared memory book segments for all pairs referenced in an
 * instrument response.
 */
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

/**
 * Update shared memory representation of level book.
 */
void shmem_sink_t::accept(const response::book_t &response,
                          const model::level_book_t &level_book) {
  const auto &symbol = response.symbol();

  const model::sides_t &sides = level_book.sides(symbol);

  auto it = m_book_segments.find(symbol);
  if (it == m_book_segments.end()) {
    const auto message = "unknown symbol: " + symbol;
    throw std::runtime_error(message);
  }
  book_segment_t &segment = *(it->second);
  segment.accept(sides);
}

} // namespace shmem
} // namespace kdr
