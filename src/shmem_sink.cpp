#include "shmem_sink.hpp"

#include <boost/log/trivial.hpp>

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
  m_price_precision = sides.price_precision();
  m_qty_precision = sides.qty_precision();
  update_side(m_bids, m_num_bids, sides.bids());
  update_side(m_asks, m_num_asks, sides.asks());
}

boost::json::object book_content_t::to_json_obj() const {
  auto bid_objs = boost::json::array{};
  for (size_t idx = 0; idx < m_num_bids; ++idx) {
    const quote_t &quote = m_bids[idx];
    bid_objs.push_back(
        boost::json::object{{quote.first.str(m_price_precision),
                             quote.second.str(m_qty_precision)}});
  }
  auto ask_objs = boost::json::array{};
  for (size_t idx = 0; idx < m_num_asks; ++idx) {
    const quote_t &quote = m_asks[idx];
    ask_objs.push_back(
        boost::json::object{{quote.first.str(m_price_precision),
                             quote.second.str(m_qty_precision)}});
  }

  boost::json::object result = {{"price_precision", m_price_precision},
                                {"qty_precision", m_qty_precision},
                                {response::book_t::c_bids, bid_objs},
                                {response::book_t::c_asks, ask_objs}};
  return result;
}

/******************************************************************************/
/**                                                                          **/
/**  t r a d e _ c o n t e n t _ t                                           **/
/**                                                                          **/
/******************************************************************************/

void trade_content_t::accept(const model::trade_t &trade) {
  m_ord_type = trade.ord_type();
  m_price = trade.price();
  m_qty = trade.qty();
  m_side = trade.side();
  m_timestamp = trade.timestamp().micros();
  m_trade_id = trade.trade_id();
}

boost::json::object trade_content_t::to_json_obj() const {
  boost::json::object result = {
      {model::trade_t::c_ord_type, model::ord_type_t_to_str(m_ord_type)},
      {model::trade_t::c_price, m_price.str()},
      {model::trade_t::c_qty, m_qty.str()},
      {model::trade_t::c_side, model::side_t_to_str(m_side)},
      {model::trade_t::c_timestamp, m_timestamp},
      {model::trade_t::c_trade_id, m_trade_id}};
  return result;
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

book_segment_t::book_segment_t(const shmem_names_t &names)
    : m_segment_remover{names},
      m_segment{bip::create_only, m_segment_remover.name().c_str(),
                c_book_segment_size},
      m_name(names.segment()),
      m_content{m_segment.construct<book_content_t>(names.content().c_str())()},
      m_mutex_remover(names),
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
/**  t r a d e _ s e g m e n t _ t                                           **/
/**                                                                          **/
/******************************************************************************/

static const size_t c_trade_segment_size =
    sizeof(shmem::trade_content_t) +
    (c_page_size - sizeof(shmem::trade_content_t) % c_page_size);

trade_segment_t::trade_segment_t(const shmem_names_t &names)
    : m_segment_remover{names},
      m_segment{bip::create_only, m_segment_remover.name().c_str(),
                c_trade_segment_size},
      m_name(names.segment()), m_content{m_segment.construct<trade_content_t>(
                                   names.content().c_str())()},
      m_mutex_remover(names),
      m_mutex{bip::create_only, m_mutex_remover.name().c_str()} {
  BOOST_LOG_TRIVIAL(debug) << __FUNCTION__ << " created segment: " << m_name;
}

trade_segment_t::~trade_segment_t() {
  if (m_content) {
    m_segment.destroy<trade_content_t>(m_name.c_str());
  }
}

void trade_segment_t::accept(const model::trade_t &trade) {
  if (!m_content) {
    const auto message =
        std::string(__FUNCTION__) + " unexpected null_ptr m_content";
    throw std::runtime_error(message);
  }

  bip::scoped_lock<bip::named_mutex> lock;
  m_content->accept(trade);
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

  BOOST_LOG_TRIVIAL(debug) << __FUNCTION__
                           << " c_trade_segment_size: " << c_trade_segment_size
                           << " sizeof(trade_content_t): "
                           << sizeof(trade_content_t)
                           << " alignof(trade_content_t): "
                           << alignof(trade_content_t);

  for (const model::pair_t &pair : response.pairs()) {
    const std::string &symbol{pair.symbol()};

    const auto bit = m_book_segments.find(symbol);
    if (bit == m_book_segments.end()) {
      const shmem_names_t shmem_names{symbol,
                                      std::string{shmem_names_t::c_book_kind}};
      book_segment_ptr book_segment =
          std::make_unique<book_segment_t>(shmem_names);
      m_book_segments.insert(std::make_pair(symbol, std::move(book_segment)));
    }

    const auto tit = m_trade_segments.find(symbol);
    if (tit == m_trade_segments.end()) {
      const shmem_names_t shmem_names{symbol,
                                      std::string{shmem_names_t::c_trade_kind}};
      trade_segment_ptr trade_segment =
          std::make_unique<trade_segment_t>(shmem_names);
      m_trade_segments.insert(std::make_pair(symbol, std::move(trade_segment)));
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

void shmem_sink_t::accept(const kdr::response::trades_t &response) {
  for (const model::trade_t &trade : response) {
    const auto &symbol = trade.symbol();
    auto it = m_trade_segments.find(symbol);
    if (it == m_trade_segments.end()) {
      const auto message = "unknown symbol: " + symbol;
      throw std::runtime_error(message);
    }
    trade_segment_t &segment = *(it->second);
    segment.accept(trade);
  }
}

} // namespace shmem
} // namespace kdr
