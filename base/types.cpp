#include "types.hpp"

#include <date/date.h>
#include <date/tz.h>

#include <chrono>
#include <iomanip>
#include <sstream>

namespace krakpot {

int64_t timestamp_t::from_iso_8601(const std::string &buffer) {
  std::istringstream in{buffer};
  date::sys_time<std::chrono::microseconds> tp;
  in >> date::parse("%FT%TZ", tp);
  const auto microseconds_since_epoch = tp.time_since_epoch().count();
  return microseconds_since_epoch;
}

std::string timestamp_t::to_iso_8601(int64_t micros) {
  using std::chrono::duration_cast;
  using std::chrono::microseconds;
  using std::chrono::system_clock;
  using std::chrono::time_point;

  const auto micros_since_epoch = microseconds{micros};
  system_clock::time_point tp(
      duration_cast<system_clock::time_point::duration>(micros_since_epoch));
  std::ostringstream iso_time_ss;
  auto gmt_tp = date::make_zoned("GMT", tp);
  iso_time_ss << date::format("%FT%TZ", gmt_tp);
  return iso_time_ss.str();
}

timestamp_t timestamp_t::now() {
  const int64_t micros{std::chrono::duration_cast<std::chrono::microseconds>(
                           std::chrono::system_clock::now().time_since_epoch())
                           .count()};
  return timestamp_t{micros};
}

} // namespace krakpot
