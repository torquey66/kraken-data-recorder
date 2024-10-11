#pragma once

#include <boost/format.hpp>

#include <ranges>
#include <string>

namespace kdr {
namespace shmem {

struct shmem_names_t final {

  static constexpr char c_sep = '_';
  static constexpr std::string_view c_prefix = "kdr";
  static constexpr std::string_view c_segment = "segment";
  static constexpr std::string_view c_content = "content";
  static constexpr std::string_view c_mutex = "mutex";
  static constexpr std::string_view c_format = "%1%_%2%_%3%_%4%";

  static constexpr std::string_view c_book_kind = "book";
  static constexpr std::string_view c_trade_kind = "trade";

  shmem_names_t(std::string symbol, std::string kind);

  std::string normalized() const { return m_normalized; };
  std::string segment() const { return m_segment; };
  std::string content() const { return m_content; };
  std::string mutex() const { return m_mutex; }

private:
  static std::string normalize(std::string symbol);

  const std::string m_normalized;
  const std::string m_segment;
  const std::string m_content;
  const std::string m_mutex;
};

inline shmem_names_t::shmem_names_t(std::string symbol, std::string kind)
    : m_normalized{normalize(symbol)},
      m_segment{boost::str(boost::format(std::string{c_format}) % c_prefix %
                           kind % c_segment % m_normalized)},
      m_content{boost::str(boost::format(std::string{c_format}) % c_prefix %
                           kind % c_content % m_normalized)},
      m_mutex{boost::str(boost::format(std::string{c_format}) % c_prefix %
                         kind % c_mutex % m_normalized)} {}

inline std::string shmem_names_t::normalize(std::string symbol) {
  const auto normalized = symbol | std::views::transform([](char ch) {
                            return ch == '/' ? c_sep : ch;
                          });
  const std::string result(normalized.begin(), normalized.end());
  return result;
}

} // namespace shmem
} // namespace kdr
