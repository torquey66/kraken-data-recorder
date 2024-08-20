#pragma once

#include <date/date.h>
#include <date/tz.h>

#include <chrono>
#include <cstdint>
#include <iomanip>
#include <sstream>
#include <string>

namespace kdr {

/**
 * AFAIK, Kraken timetamps are always ISO 8601 strings in GMT.
 *
 * TODO: consider using chrono timepoints in some fashion instead of
 * raw micros.
 */
struct timestamp_t final {
  timestamp_t() {}
  timestamp_t(int64_t in_micros) : m_micros{in_micros} {}

  template <typename S>
  timestamp_t(S str) : m_micros{from_iso_8601(str)} {}

  auto operator<=>(const timestamp_t&) const = default;

  std::string str() const { return to_iso_8601(m_micros); };

  int64_t micros() const { return m_micros; }

  template <typename S>
  static int64_t from_iso_8601(S buffer);

  static int64_t from_iso_8601(std::string_view view) {
    return from_iso_8601(std::string{view.data(), view.size()});
  }

  static std::string to_iso_8601(int64_t micros);

  static timestamp_t now();

 private:
  int64_t m_micros = 0;
};

/******************************************************************************/

template <typename S>
int64_t timestamp_t::from_iso_8601(S buffer) {
  std::istringstream ins{buffer};
  date::sys_time<std::chrono::microseconds> tsp;
  ins >> date::parse("%FT%TZ", tsp);
  const auto microseconds_since_epoch = tsp.time_since_epoch().count();
  return microseconds_since_epoch;
}

inline std::string timestamp_t::to_iso_8601(int64_t micros) {
  using std::chrono::duration_cast;
  using std::chrono::microseconds;
  using std::chrono::system_clock;
  using std::chrono::time_point;

  const auto micros_since_epoch = microseconds{micros};
  const system_clock::time_point mse_tp(
      duration_cast<system_clock::time_point::duration>(micros_since_epoch));
  std::ostringstream iso_time_ss;
  auto gmt_tp = date::make_zoned("GMT", mse_tp);
  iso_time_ss << date::format("%FT%TZ", gmt_tp);
  return iso_time_ss.str();
}

inline timestamp_t timestamp_t::now() {
  const int64_t micros{std::chrono::duration_cast<std::chrono::microseconds>(
                           std::chrono::system_clock::now().time_since_epoch())
                           .count()};
  return timestamp_t{micros};
}

}  // namespace kdr
