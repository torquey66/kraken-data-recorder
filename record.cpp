#include "record.hpp"

#include <sstream>

namespace krakpot {

void record_t::reset() {
  channel_id = BAD_CHANNEL_ID;
  as.clear();
  bs.clear();
  a.clear();
  b.clear();
  c = 0;
  channel_name.clear();
  pair.clear();
}

std::string record_t::to_string() const {
  auto os = std::ostringstream();
  os << "id: " << channel_id << " num as: " << as.size()
     << " num bs: " << bs.size() << " num a: " << a.size()
     << " num b: " << b.size() << " c: " << c << " name: " << channel_name
     << " pair: " << pair;
  return os.str();
}

} // namespace krakpot
