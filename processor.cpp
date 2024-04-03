#include "processor.hpp"

#include <string_view>

namespace krakpot {

void processor_t::process(std::string_view msg) {
  // !@# TODO: figure out how to eliminate this copy and still meet
  // simdjson's padding needs.
  simdjson::padded_string padded_msg{msg};
  simdjson::ondemand::document doc = m_parser.iterate(padded_msg);
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
  std::string buffer; // !@#
  for (auto value : src) {
    switch (idx) {
    case 0:
      copy_string(value, buffer);
      dst.price = decimal_t{buffer};
      break;
    case 1:
      copy_string(value, buffer);
      dst.volume = decimal_t{buffer};
      break;
    case 2:
      copy_string(value, dst.timestamp);
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
            m_record.c = src.get_uint64_in_string();
            /*
            error = copy_string(src, m_record.c);
            if (error) {
              return error;
            }
            */
          }
        }
      } else {
        BOOST_LOG_TRIVIAL(error) << "type: " << value.type();
        return simdjson::INCORRECT_TYPE;
      }
    }

    if (m_record.is_snapshot()) {
      BOOST_LOG_TRIVIAL(info) << "snapshot: " << m_record.to_string();
      m_books[m_record.pair] = book_t{m_record};
    } else {
      BOOST_LOG_TRIVIAL(info) << "  update: " << m_record.to_string();
      m_books[m_record.pair].update(m_record);
    }
  }
  return error;
}

} // namespace krakpot
