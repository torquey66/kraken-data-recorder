cmake_minimum_required(VERSION 3.29.2)

project(KrakpotProject)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_EXTENSIONS ON)

set(CMAKE_OSX_DEPLOYMENT_TARGET "10.13" CACHE STRING "Minimum OS X deployment version")

#add_compile_options(-fsanitize=address)
#add_link_options(-fsanitize=address)

add_compile_options(-Wall -Wextra -Wpedantic -Werror)

include(${CMAKE_BINARY_DIR}/conan_toolchain.cmake)

find_package(Arrow)                                                                                                                          
find_package(Boost)                                                                                                                          
find_package(date)
find_package(doctest)                                                                                                                        
find_package(OpenSSL)                                                                                                                        
find_package(simdjson)                                                                                                                       

# TODO: split these into 'src' and 'include' directories or the like
include_directories(${CMAKE_SOURCE_DIR}/base)
add_library(krakpot_base
  base/asset.cpp
  base/config.cpp
  base/constants.hpp
  base/decimal.cpp
  base/decimal.hpp
  base/engine.cpp
  base/level_book.cpp
  base/metrics.cpp
  base/pair.cpp
  base/refdata.cpp
  base/refdata.hpp
  base/requests.cpp
  base/responses.cpp
  base/responses.hpp
  base/session.cpp
  base/sink.hpp
  base/types.cpp
)

# TODO: split these into 'src' and 'include' directories or the like
include_directories(${CMAKE_SOURCE_DIR}/parquet)
add_library(krakpot_parquet
  parquet/assets_sink.cpp
  parquet/assets_sink.hpp
  parquet/book_sink.cpp
  parquet/book_sink.hpp
  parquet/io.hpp
  parquet/pairs_sink.cpp
  parquet/pairs_sink.hpp
  parquet/trade_sink.cpp
  parquet/trade_sink.hpp
)

target_link_libraries(krakpot_base
  PUBLIC
  arrow::arrow
  boost::boost
  date::date
  openssl::openssl
  simdjson::simdjson
)

target_link_libraries(krakpot_parquet
  PUBLIC
  arrow::arrow
  krakpot_base
)

add_executable(kp_record kp_record/kp_record.cpp)
target_link_libraries(kp_record krakpot_parquet)

##
# Manual tests
# 
add_executable(parse_instrument_snapshot test/parse_instrument_snapshot.cpp)
target_link_libraries(parse_instrument_snapshot krakpot_base)

add_executable(check_book test/check_book.cpp)
target_link_libraries(check_book krakpot_base)

##
# Unit tests
#
enable_testing()

add_executable(tests
  test/unit/decimal_test.cpp
  test/unit/json_test.cpp
  test/unit/level_book_test.cpp
  test/unit/parquet_test.cpp
  test/unit/response_test.cpp
  test/unit/test_main.cpp
)
target_link_libraries(tests krakpot_base doctest::doctest)

add_test(NAME unit_test COMMAND tests)
