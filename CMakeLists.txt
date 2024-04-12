cmake_minimum_required(VERSION 3.16)
project(KrakpotProject)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_EXTENSIONS ON)

add_compile_options(-fsanitize=address)
add_link_options(-fsanitize=address)

add_compile_options(-Wall -Wextra -Wpedantic -Werror)

include(${CMAKE_BINARY_DIR}/conan_toolchain.cmake)

find_package(Arrow)                                                                                                                          
find_package(Boost)                                                                                                                          
find_package(doctest)                                                                                                                        
find_package(nlohmann_json)                                                                                                                  
find_package(OpenSSL)                                                                                                                        
find_package(simdjson)                                                                                                                       

# TODO: move most of this code into a library and link against it below
add_executable(Krakpot
  asset.cpp
  cert.cpp
  engine.cpp
  main.cpp
  pair.cpp
  parquet.cpp
  requests.cpp
  responses.cpp
  session.cpp
)

target_link_libraries(Krakpot
  arrow::arrow
  boost::boost
  doctest::doctest
  nlohmann_json::nlohmann_json
  openssl::openssl
  simdjson::simdjson
)

add_executable(parse_instrument_snapshot
  asset.cpp
  pair.cpp
  responses.cpp
  test/parse_instrument_snapshot.cpp
)

target_link_libraries(parse_instrument_snapshot
  nlohmann_json::nlohmann_json
  simdjson::simdjson
)

add_executable(response_test
  asset.cpp
  pair.cpp
  responses.cpp
  test/unit/response_test.cpp
)

target_link_libraries(response_test
  doctest::doctest
  nlohmann_json::nlohmann_json
  simdjson::simdjson
)

