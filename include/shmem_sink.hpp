#pragma once

#include "book.hpp"
#include "depth.hpp"
#include "instrument.hpp"
#include "level_book.hpp"
#include "shmem_names.hpp"
#include "trades.hpp"
#include "types.hpp"

#include <boost/interprocess/managed_shared_memory.hpp>
#include <boost/interprocess/sync/named_mutex.hpp>
#include <boost/interprocess/sync/scoped_lock.hpp>
#include <boost/json.hpp>

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

  book_content_t() {}
  book_content_t(const book_content_t &) = default;
  book_content_t &operator=(const book_content_t &) = default;

  size_t num_bids() const { return m_num_bids; }
  const kdr::quote_t &bid(size_t idx) const { return m_bids.at(idx); }

  size_t num_asks() const { return m_num_asks; }
  const kdr::quote_t &ask(size_t idx) const { return m_asks.at(idx); }

  void accept(const model::sides_t &sides);

  boost::json::object to_json_obj() const;
  std::string str() const { return boost::json::serialize(to_json_obj()); }

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

  integer_t m_price_precision = 0;
  integer_t m_qty_precision = 0;
  size_t m_num_bids = 0;
  size_t m_num_asks = 0;
  std::array<kdr::quote_t, c_max_depth> m_bids;
  std::array<kdr::quote_t, c_max_depth> m_asks;
};

/**
 * Shared memory representation of trade state.
 */
struct trade_content_t final {
  trade_content_t() {}
  trade_content_t(const trade_content_t &) = default;
  trade_content_t &operator=(const trade_content_t &) = default;

  void accept(const model::trade_t &trade);

  boost::json::object to_json_obj() const;
  std::string str() const { return boost::json::serialize(to_json_obj()); }

private:
  model::ord_type_t m_ord_type = model::ord_type_invalid;
  price_t m_price;
  qty_t m_qty;
  model::side_t m_side = model::side_invalid;
  integer_t m_timestamp = 0;
  integer_t m_trade_id = 0;
};

/**
 * RAII wrapper of named shared memory object.
 */
struct segment_remover_t final {

  segment_remover_t(const shmem_names_t &names) : m_name{names.segment()} {
    bip::shared_memory_object::remove(m_name.c_str());
  }

  ~segment_remover_t() { bip::shared_memory_object::remove(m_name.c_str()); }

  const std::string &name() const { return m_name; }

private:
  const std::string m_name;
};

/**
 * RAII wrapper of named mutex.
 */
struct mutex_remover_t final {

  mutex_remover_t(const shmem_names_t &names) : m_name{names.mutex()} {
    bip::named_mutex::remove(m_name.c_str());
  }

  ~mutex_remover_t() { bip::named_mutex::remove(m_name.c_str()); }

  const std::string &name() const { return m_name; }

private:
  const std::string m_name;
};

/**
 * RAII wrapper of book shared memory segment, etc.
 */
struct book_segment_t final {

  book_segment_t(const shmem_names_t &names);
  ~book_segment_t();

  void accept(const model::sides_t &sides);

private:
  segment_remover_t m_segment_remover;
  bip::managed_shared_memory m_segment;
  const std::string m_name;
  book_content_t *m_content = nullptr;
  mutex_remover_t m_mutex_remover;
  bip::named_mutex m_mutex;
};

/**
 * RAII wrapper of trade shared memory segment, etc.
 */
struct trade_segment_t final {

  trade_segment_t(const shmem_names_t &names);
  ~trade_segment_t();

  void accept(const model::trade_t &trade);

private:
  segment_remover_t m_segment_remover;
  bip::managed_shared_memory m_segment;
  const std::string m_name;
  trade_content_t *m_content = nullptr;
  mutex_remover_t m_mutex_remover;
  bip::named_mutex m_mutex;
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

  /**
   * Update shared memory representation of trades.
   */
  void accept(const kdr::response::trades_t &response);

private:
  using book_segment_ptr = std::unique_ptr<book_segment_t>;
  using trade_segment_ptr = std::unique_ptr<trade_segment_t>;

  std::unordered_map<std::string, book_segment_ptr> m_book_segments;
  std::unordered_map<std::string, trade_segment_ptr> m_trade_segments;
};

} // namespace shmem
} // namespace kdr
