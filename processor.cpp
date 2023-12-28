#include "processor.hpp"

#include <string_view>

namespace krakpot {

void processor_t::process(std::string &msg) {
  //  BOOST_LOG_TRIVIAL(debug) << msg;
  simdjson::ondemand::document doc = m_parser.iterate(msg);
  auto error = process_book_msg(doc);
  if (error) {
    BOOST_LOG_TRIVIAL(debug) << simdjson::to_json_string(doc);
  }
}

template <typename V>
simdjson::error_code copy_string(V &src, std::string &dst) {
  auto sv = std::string_view{};
  const auto error = src.get_string().get(sv);
  if (!error) {
    dst = std::string(sv.data(), sv.size());
  }
  return error;
}

template <typename V> simdjson::error_code copy_entry(V &src, entry_t &dst) {
  // !@# UGH - there has got to be a better way...
  dst = entry_t{};
  auto idx = 0;
  for (auto value : src) {
    switch (idx) {
    case 0:
      dst.price = value.get_double_in_string();
      break;
    case 1:
      dst.volume = value.get_double_in_string();
      break;
    case 2:
      dst.timestamp = value.get_double_in_string();
      break;
    case 3:
      copy_string(value, dst.update_type);
    }
    ++idx;
  }
  return simdjson::SUCCESS; // !@# fix error handling
}

template <typename V>
simdjson::error_code copy_entries(V &src, std::vector<entry_t> &dst) {
  // !@# UGH - there has got to be a better way...
  auto entries = src.get_array();
  for (auto entry : entries) {
    dst.emplace_back();
    copy_entry(entry, dst.back());
  }
  return simdjson::SUCCESS; // !@# fix error handling
}

simdjson::error_code
processor_t::process_book_msg(simdjson::ondemand::document &doc) {
  simdjson::ondemand::array book_msg;
  auto error = doc.get_array().get(book_msg);
  if (!error) {
    m_record.reset();

    for (auto value : book_msg) {
      if (value.type() == simdjson::ondemand::json_type::number) {
        error = value.get_int64().get(m_record.channel_id);
        if (error) {
          return error;
        }
      } else if (value.type() == simdjson::ondemand::json_type::string) {
        if (m_record.channel_name.empty()) {
          error = copy_string(value, m_record.channel_name);
          if (error) {
            return error;
          }
        } else if (m_record.pair.empty()) {
          error = copy_string(value, m_record.pair);
          if (error) {
            return error;
          }
        } else {
          return simdjson::UNEXPECTED_ERROR;
        }
      } else if (value.type() == simdjson::ondemand::json_type::object) {
        auto object = value.get_object();
        for (auto field : object) {
          const auto key = field.key();
          auto src = field.value();
          if (field.key() == "a") {
            copy_entries(src, m_record.a);
          } else if (field.key() == "b") {
            copy_entries(src, m_record.b);
          } else if (field.key() == "as") {
            copy_entries(src, m_record.as);
          } else if (field.key() == "bs") {
            copy_entries(src, m_record.bs);
          } else if (field.key() == "c") {
            error = copy_string(src, m_record.c);
            if (error) {
              return error;
            }
          }
        }
      } else {
        BOOST_LOG_TRIVIAL(error) << "type: " << value.type();
        return simdjson::INCORRECT_TYPE;
      }
    }

    BOOST_LOG_TRIVIAL(debug) << m_record.to_string();
  }
  return error;
}

} // namespace krakpot
