#pragma once

#include <boost/log/trivial.hpp>
#include <simdjson.h>

#include <string>

namespace krakpot {

struct processor_t final {

  void process(std::string &);

private:
  simdjson::error_code process_book_msg(simdjson::ondemand::document &);

  simdjson::ondemand::parser m_parser;
};

} // namespace krakpot
