cmake_minimum_required(VERSION 3.16)
project(KrakpotProject)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_EXTENSIONS ON)

#add_compile_options(-fsanitize=address)
#add_link_options(-fsanitize=address)

add_compile_options(-Wall -Wextra -Wpedantic -Werror)

include(${CMAKE_BINARY_DIR}/conan_toolchain.cmake)

find_package(Arrow)                                                                                                                          
find_package(Boost)                                                                                                                          
find_package(date)
find_package(doctest)                                                                                                                        
find_package(nlohmann_json)                                                                                                                  
find_package(OpenSSL)                                                                                                                        
find_package(simdjson)                                                                                                                       

# TODO: split these into 'src' and 'include' directories or the like
include_directories(${CMAKE_SOURCE_DIR})
add_library(krakpot_base
  asset.cpp
  engine.cpp
  level_book.cpp
  pair.cpp
  parquet.cpp
  requests.cpp
  responses.cpp
  session.cpp
  types.cpp
  responses.hpp
  parquet.hpp
)

target_link_libraries(krakpot_base
  PUBLIC
  arrow::arrow
  boost::boost
  date::date
  doctest::doctest
  nlohmann_json::nlohmann_json
  openssl::openssl
  simdjson::simdjson
)

add_executable(Krakpot main.cpp)
target_link_libraries(Krakpot krakpot_base)

add_executable(parse_instrument_snapshot test/parse_instrument_snapshot.cpp)
target_link_libraries(parse_instrument_snapshot krakpot_base)

add_executable(parquet_test test/unit/parquet_test.cpp)
target_link_libraries(parquet_test krakpot_base)

add_executable(response_test test/unit/response_test.cpp)
target_link_libraries(response_test krakpot_base)

