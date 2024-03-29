#include "../pair.hpp" // !@# TODO: fix CMake include to eliminate relative path

#include <iostream>
#include <simdjson.h>

int main(int argc, char *argv[]) {
  if (argc != 2) {
    std::cerr << "usage: " << argv[0] << " <json file>" << std::endl;
    return -1;
  }
  const auto snapshot_file = std::string{argv[1]};
  try {
    auto parser = simdjson::ondemand::parser{};
    auto json = simdjson::padded_string::load(snapshot_file);
    simdjson::ondemand::document doc = parser.iterate(json);
    for (simdjson::fallback::ondemand::object obj : doc["data"]["pairs"]) {
      const auto pair = krakpot::pair_t::from_json(obj);
      std::cout << pair.str() << std::endl;
    }
  } catch (const std::exception &ex) {
    std::cerr << ex.what() << std::endl;
    return -1;
  }
}
