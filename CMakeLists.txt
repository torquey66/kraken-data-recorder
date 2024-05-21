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
include_directories(${CMAKE_SOURCE_DIR}/base)
add_library(krakpot_base
  base/asset.cpp
  base/config.cpp
  base/decimal.cpp
  base/engine.cpp
  base/level_book.cpp
  base/metrics.cpp
  base/pair.cpp
  base/parquet.cpp
  base/requests.cpp
  base/responses.cpp
  base/session.cpp
  base/types.cpp
  base/decimal.hpp
  base/responses.hpp
  base/parquet.hpp
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

add_executable(check_book test/check_book.cpp)
target_link_libraries(check_book krakpot_base)

add_executable(decimal_test test/unit/decimal_test.cpp)
target_link_libraries(decimal_test krakpot_base)

add_executable(level_book_test test/unit/level_book_test.cpp)
target_link_libraries(level_book_test krakpot_base)

add_executable(parquet_test test/unit/parquet_test.cpp)
target_link_libraries(parquet_test krakpot_base)

add_executable(response_test test/unit/response_test.cpp)
target_link_libraries(response_test krakpot_base)

