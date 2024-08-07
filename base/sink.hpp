#pragma once

#include "responses.hpp"

#include <functional>

namespace kdr {

struct sink_t final {
  using accept_instrument_t =
      std::function<void(const response::instrument_t&)>;
  using accept_book_t = std::function<void(const response::book_t&)>;
  using accept_trades_t = std::function<void(const response::trades_t&)>;

  sink_t(const accept_instrument_t& accept_instrument,
         const accept_book_t& accept_book,
         const accept_trades_t& accept_trades)
      : m_accept_instrument{accept_instrument},
        m_accept_book(accept_book),
        m_accept_trades(accept_trades) {}

  void accept(const response::instrument_t& response) const {
    m_accept_instrument(response);
  }

  void accept(const response::book_t& response) const {
    m_accept_book(response);
  }

  void accept(const response::trades_t& response) const {
    m_accept_trades(response);
  }

 private:
  accept_instrument_t m_accept_instrument;
  accept_book_t m_accept_book;
  accept_trades_t m_accept_trades;
};

}  // namespace kdr
