#include "instrument.hpp"

#include <simdjson.h>
#include <iostream>

int main(int argc, char* argv[]) {
  if (argc != 2) {
    std::cerr << "usage: " << argv[0] << " <json file>" << std::endl;
    return -1;
  }
  const auto snapshot_file = std::string{argv[1]};
  try {
    auto parser = simdjson::ondemand::parser{};
    auto json = simdjson::padded_string::load(snapshot_file);
    simdjson::ondemand::document doc = parser.iterate(json);
    const auto response = kdr::response::instrument_t::from_json(doc);
    std::cout << response.str() << std::endl;
  } catch (const std::exception& ex) {
    std::cerr << ex.what() << std::endl;
    return -1;
  }
}
