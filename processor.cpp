#include "processor.hpp"

#include <variant>

namespace krakpot {

using channel_id_t = int64_t;
using channel_name_t = std::string;
using pair_t = std::string;

struct snapshot_t final {
  channel_id_t channel_id;
  channel_name_t channel_name;
  pair_t pair;
};

struct update_t final {
  channel_id_t channel_id;
  channel_name_t channel_name;
  pair_t pair;
};

using book_msg_t = std::variant<snapshot_t, update_t>;

void processor_t::process(std::string &msg) {
  simdjson::ondemand::document doc = m_parser.iterate(msg);
  auto error = process_book_msg(doc);
  if (error) {
    BOOST_LOG_TRIVIAL(debug) << simdjson::to_json_string(doc);
  }
}

simdjson::error_code
processor_t::process_book_msg(simdjson::ondemand::document &doc) {
  simdjson::ondemand::array book_msg;
  auto error = doc.get_array().get(book_msg);
  if (!error) {
    for (auto value : book_msg) {
      BOOST_LOG_TRIVIAL(debug) << value.type();
      if (value.type() == simdjson::ondemand::json_type::string) {
        BOOST_LOG_TRIVIAL(debug) << value.get_string();
      } else if (value.type() == simdjson::ondemand::json_type::number) {
        BOOST_LOG_TRIVIAL(debug) << value.get_int64();
      } else if (value.type() == simdjson::ondemand::json_type::object) {
        auto object = value.get_object();
        for (auto field : object) {
          BOOST_LOG_TRIVIAL(debug) << "   " << field.key();
        }
      } else {
        return simdjson::INCORRECT_TYPE;
      }
    }
  }
  return error;
}

} // namespace krakpot
