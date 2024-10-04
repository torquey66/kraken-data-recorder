#include "shmem_sink.hpp"

#include <cstdlib>

namespace kdr {
namespace shmem {

/******************************************************************************/
/**                                                                          **/
/**  b o o k _ c o n t e n t _ t                                             **/
/**                                                                          **/
/******************************************************************************/

void book_content_t::accept(const model::sides_t &sides) {
  if (sides.book_depth() < 0) {
    const auto message = std::string(__FUNCTION__) + " invalid book_depth: " +
                         std::to_string(sides.book_depth());
    throw std::runtime_error(message);
  }
  if (static_cast<size_t>(sides.book_depth()) > c_max_depth) {
    const auto message = std::string(__FUNCTION__) +
                         " book_depth: " + std::to_string(sides.book_depth()) +
                         " exceeds c_max_depth: " + std::to_string(c_max_depth);
    throw std::runtime_error(message);
  }
  update_side(m_bids, m_num_bids, sides.bids());
  update_side(m_asks, m_num_asks, sides.asks());
}

/******************************************************************************/
/**                                                                          **/
/**  b o o k _ s e g m e n t _ t                                             **/
/**                                                                          **/
/******************************************************************************/

static const size_t c_page_size = sysconf(_SC_PAGE_SIZE);
static const size_t c_book_segment_size =
    sizeof(book_content_t) +
    (c_page_size - sizeof(book_content_t) % c_page_size);

book_segment_t::book_segment_t(std::string symbol)
    : m_book_obj{symbol},
      m_segment{boost::interprocess::create_only, m_book_obj.obj_name().c_str(),
                c_book_segment_size},
      m_content{m_segment.construct<book_content_t>(m_name.c_str())()},
      m_book_mutex(symbol), m_mutex{boost::interprocess::create_only,
                                    m_book_mutex.mutex_name().c_str()} {
  BOOST_LOG_TRIVIAL(debug) << __FUNCTION__
                           << " created segment for symbol: " << symbol;
}

book_segment_t::~book_segment_t() {
  if (m_content) {
    m_segment.destroy<book_content_t>(m_name.c_str());
  }
}

void book_segment_t::accept(const model::sides_t &sides) {
  if (!m_content) {
    const auto message =
        std::string(__FUNCTION__) + " unexpected null_ptr m_content";
    throw std::runtime_error(message);
  }

  boost::interprocess::scoped_lock<boost::interprocess::named_mutex> lock;
  m_content->accept(sides);
}

/******************************************************************************/
/**                                                                          **/
/** s h m e m _ s i n k _ t                                                  **/
/**                                                                          **/
/******************************************************************************/

void shmem_sink_t::accept(const response::instrument_t &response) {
  BOOST_LOG_TRIVIAL(debug) << __FUNCTION__
                           << " c_book_segment_size: " << c_book_segment_size
                           << " sizeof(book_content_t): "
                           << sizeof(book_content_t)
                           << " alignof(book_content_t): "
                           << alignof(book_content_t);

  for (const model::pair_t &pair : response.pairs()) {
    const auto it = m_book_segments.find(pair.symbol());
    if (it == m_book_segments.end()) {
      book_segment_ptr book_segment =
          std::make_unique<book_segment_t>(segment_name(pair.symbol()));
      m_book_segments.insert(
          std::make_pair(pair.symbol(), std::move(book_segment)));
    }
  }
}

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

std::string shmem_sink_t::segment_name(std::string symbol) {
  const auto replaced = symbol | std::views::transform([](char ch) {
                          return ch == '/' ? '_' : ch;
                        });
  const std::string result(replaced.begin(), replaced.end());
  return result;
}

} // namespace shmem
} // namespace kdr
