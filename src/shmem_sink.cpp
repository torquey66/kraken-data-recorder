#include "shmem_sink.hpp"

#include <boost/log/trivial.hpp>

#include <cstdlib>

namespace kdr {
namespace shmem {

/******************************************************************************/
/**                                                                          **/
/**  n a m i n g  u t i l i t i e s                                          **/
/**                                                                          **/
/******************************************************************************/
std::string normalized_symbol(std::string symbol) {
  const auto replaced = symbol | std::views::transform([](char ch) {
                          return ch == '/' ? '_' : ch;
                        });
  const std::string result(replaced.begin(), replaced.end());
  return result;
}

std::string segment_name(std::string suffix) {
  return "kdr_book_segment_t_" + suffix;
}

std::string content_name(std::string suffix) {
  return "kdr_book_content_t_" + suffix;
}

std::string mutex_name(std::string suffix) {
  return "kdr_book_mutex_t_" + suffix;
}

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
  m_price_precision = sides.price_precision();
  m_qty_precision = sides.qty_precision();
  update_side(m_bids, m_num_bids, sides.bids());
  update_side(m_asks, m_num_asks, sides.asks());
}

boost::json::object book_content_t::to_json_obj() const {
  return boost::json::object{};
}

/******************************************************************************/
/**                                                                          **/
/**  b o o k _ s e g m e n t _ t                                             **/
/**                                                                          **/
/******************************************************************************/

static const size_t c_page_size = sysconf(_SC_PAGE_SIZE);
static const size_t c_book_segment_size =
    sizeof(shmem::book_content_t) +
    (c_page_size - sizeof(shmem::book_content_t) % c_page_size);

book_segment_t::book_segment_t(std::string suffix)
    : m_segment_remover{suffix},
      m_segment{bip::create_only, m_segment_remover.name().c_str(),
                c_book_segment_size},
      m_content{
          m_segment.construct<book_content_t>(content_name(suffix).c_str())()},
      m_mutex_remover(suffix),
      m_mutex{bip::create_only, m_mutex_remover.name().c_str()} {
  BOOST_LOG_TRIVIAL(debug) << __FUNCTION__ << " created segment: " << m_name;
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

  bip::scoped_lock<bip::named_mutex> lock;
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
          std::make_unique<book_segment_t>(normalized_symbol(pair.symbol()));
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

} // namespace shmem
} // namespace kdr
