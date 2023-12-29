#pragma once

#include "book.hpp"
#include "record.hpp"

#include <boost/log/trivial.hpp>
#include <simdjson.h>

#include <string>
#include <unordered_map>

namespace krakpot {

struct processor_t final {

  void process(std::string &);

private:
  simdjson::error_code process_book_msg(simdjson::ondemand::document &);

  simdjson::ondemand::parser m_parser;
  record_t m_record;

  std::unordered_map<std::string, book_t> m_books;
};

} // namespace krakpot
