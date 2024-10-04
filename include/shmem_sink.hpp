#pragma once

#include "book.hpp"
#include "depth.hpp"
#include "instrument.hpp"
#include "level_book.hpp"
#include "types.hpp"

#include <boost/interprocess/managed_shared_memory.hpp>
#include <boost/interprocess/sync/named_mutex.hpp>
#include <boost/interprocess/sync/scoped_lock.hpp>
#include <boost/log/trivial.hpp>

#include <array>
#include <memory>
#include <unordered_map>

namespace kdr {
namespace shmem {

namespace bip = boost::interprocess;

/**
 * Shared memory representation of book state.
 */
struct book_content_t final {
  static constexpr size_t c_max_depth = kdr::model::depth_1000;

  size_t num_bids() const { return m_num_bids; }
  const kdr::quote_t &bid(size_t idx) const { return m_bids.at(idx); }

  size_t num_asks() const { return m_num_asks; }
  const kdr::quote_t &ask(size_t idx) const { return m_asks.at(idx); }

  void accept(const model::sides_t &sides);

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

  book_obj_t(std::string symbol) : m_obj_name{"kdr_book_obj_t_" + symbol} {
    BOOST_LOG_TRIVIAL(debug) << __FUNCTION__ << " removing: " << m_obj_name;
    bip::shared_memory_object::remove(m_obj_name.c_str());
    BOOST_LOG_TRIVIAL(debug) << __FUNCTION__ << " removed: " << m_obj_name;
  }

  ~book_obj_t() { bip::shared_memory_object::remove(m_obj_name.c_str()); }

  const std::string &obj_name() const { return m_obj_name; }

private:
  const std::string m_obj_name;
};

/**
 * RAII wrapper of named mutex.
 */
struct book_mutex_t final {

  book_mutex_t(std::string symbol)
      : m_mutex_name{"kdr_book_mutex_t_" + symbol} {
    bip::named_mutex::remove(m_mutex_name.c_str());
    BOOST_LOG_TRIVIAL(debug) << __FUNCTION__ << " removed: " << m_mutex_name;
  }

  ~book_mutex_t() { bip::named_mutex::remove(m_mutex_name.c_str()); }

  const std::string &mutex_name() const { return m_mutex_name; }

private:
  const std::string m_mutex_name;
};

/**
 * RAII wrapper of shared memory segment, etc.
 */
struct book_segment_t final {

  book_segment_t(std::string symbol);
  ~book_segment_t();

  void accept(const model::sides_t &sides);

private:
  book_obj_t m_book_obj;
  boost::interprocess::managed_shared_memory m_segment;
  const std::string m_name;
  book_content_t *m_content = nullptr;
  book_mutex_t m_book_mutex;
  boost::interprocess::named_mutex m_mutex;
};

/**
 * Shared memory sink for book and trade states.
 */
struct shmem_sink_t final {

  /**
   * Create shared memory book segments for all pairs referenced in an
   * instrument response.
   */
  void accept(const response::instrument_t &response);

  /**
   * Update shared memory representation of level book.
   */
  void accept(const response::book_t &response,
              const model::level_book_t &level_book);

private:
  static std::string segment_name(std::string symbol);

  using book_segment_ptr = std::unique_ptr<book_segment_t>;
  std::unordered_map<std::string, book_segment_ptr> m_book_segments;
};

} // namespace shmem
} // namespace kdr
