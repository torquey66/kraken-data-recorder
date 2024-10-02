cmake_minimum_required(VERSION 3.28.3)

project(KrakenDataRecorder)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_EXTENSIONS ON)

set(CMAKE_OSX_DEPLOYMENT_TARGET "10.12" CACHE STRING "Minimum OS X deployment version")

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

set(MODEL_FILES
  ${CMAKE_SOURCE_DIR}/model/asset.json
  ${CMAKE_SOURCE_DIR}/model/pair.json
  ${CMAKE_SOURCE_DIR}/model/pong.json
  ${CMAKE_SOURCE_DIR}/model/trade.json
)

set(ENUM_FILES
  ${CMAKE_SOURCE_DIR}/model/asset_status.json
  ${CMAKE_SOURCE_DIR}/model/depth.json
  ${CMAKE_SOURCE_DIR}/model/ord_type.json
  ${CMAKE_SOURCE_DIR}/model/pair_status.json
  ${CMAKE_SOURCE_DIR}/model/side.json
)

set(GENERATED_FILES
  ${CMAKE_SOURCE_DIR}/include/generated/asset.hpp
  ${CMAKE_SOURCE_DIR}/include/generated/asset_status.hpp
  ${CMAKE_SOURCE_DIR}/include/generated/depth.hpp
  ${CMAKE_SOURCE_DIR}/include/generated/ord_type.hpp
  ${CMAKE_SOURCE_DIR}/include/generated/pair.hpp
  ${CMAKE_SOURCE_DIR}/include/generated/pair_status.hpp
  ${CMAKE_SOURCE_DIR}/include/generated/pong.hpp
  ${CMAKE_SOURCE_DIR}/include/generated/side.hpp
  ${CMAKE_SOURCE_DIR}/include/generated/trade.hpp
  ${CMAKE_SOURCE_DIR}/src/generated/asset.cpp
  ${CMAKE_SOURCE_DIR}/src/generated/asset_status.cpp
  ${CMAKE_SOURCE_DIR}/src/generated/depth.cpp
  ${CMAKE_SOURCE_DIR}/src/generated/ord_type.cpp
  ${CMAKE_SOURCE_DIR}/src/generated/pair.cpp
  ${CMAKE_SOURCE_DIR}/src/generated/pair_status.cpp
  ${CMAKE_SOURCE_DIR}/src/generated/pong.cpp
  ${CMAKE_SOURCE_DIR}/src/generated/side.cpp
  ${CMAKE_SOURCE_DIR}/src/generated/trade.cpp
)

set(JINJA2_FILES
  ${CMAKE_SOURCE_DIR}/jinja2/enum_code.j2
  ${CMAKE_SOURCE_DIR}/jinja2/enum_header.j2
  ${CMAKE_SOURCE_DIR}/jinja2/model_code.j2
  ${CMAKE_SOURCE_DIR}/jinja2/model_header.j2
  ${CMAKE_SOURCE_DIR}/jinja2/types.j2
)

foreach(model_file ${MODEL_FILES})
    get_filename_component(filename ${model_file} NAME_WE)
    set(header_file ${CMAKE_SOURCE_DIR}/include/generated/${filename}.hpp)
    set(code_file ${CMAKE_SOURCE_DIR}/src/generated/${filename}.cpp)
    add_custom_command(
        OUTPUT ${header_file} ${code_file}
        COMMAND jinja2 ${CMAKE_SOURCE_DIR}/jinja2/model_header.j2 ${model_file} | clang-format --style=LLVM > ${header_file}
        COMMAND jinja2 ${CMAKE_SOURCE_DIR}/jinja2/model_code.j2   ${model_file} | clang-format --style=LLVM > ${code_file}
        DEPENDS ${model_file} ${JINJA2_FILES}
        COMMENT "Generating ${header_file} from ${model_file}"
        COMMENT "Generating ${code_file} from ${model_file}"
    )
endforeach()

foreach(enum_file ${ENUM_FILES})
    get_filename_component(filename ${enum_file} NAME_WE)
    set(header_file ${CMAKE_SOURCE_DIR}/include/generated/${filename}.hpp)
    set(code_file ${CMAKE_SOURCE_DIR}/src/generated/${filename}.cpp)
    add_custom_command(
        OUTPUT ${header_file} ${code_file}
        COMMAND jinja2 ${CMAKE_SOURCE_DIR}/jinja2/enum_header.j2 ${enum_file} | clang-format --style=LLVM > ${header_file}
        COMMAND jinja2 ${CMAKE_SOURCE_DIR}/jinja2/enum_code.j2   ${enum_file} | clang-format --style=LLVM > ${code_file}
        DEPENDS ${enum_file} ${JINJA2_FILES}
        COMMENT "Generating ${header_file} from ${enum_file}"
        COMMENT "Generating ${code_file} from ${enum_file}"
    )
endforeach()

add_custom_target(generate_all
    ALL
    DEPENDS ${GENERATED_FILES}
)

include_directories(${CMAKE_SOURCE_DIR}/include ${CMAKE_SOURCE_DIR}/include/generated)
add_library(kdr
  include/generated/asset.hpp
  include/generated/asset_status.hpp
  include/generated/depth.hpp
  include/generated/ord_type.hpp
  include/generated/pair.hpp
  include/generated/pong.hpp
  include/generated/pair_status.hpp
  include/generated/side.hpp
  include/generated/trade.hpp
  src/generated/asset.cpp
  src/generated/asset_status.cpp
  src/generated/depth.cpp
  src/generated/ord_type.cpp
  src/generated/pair.cpp
  src/generated/pair_status.cpp
  src/generated/pong.cpp
  src/generated/side.cpp
  src/generated/trade.cpp
  include/book.hpp
  include/config.hpp
  include/constants.hpp
  include/decimal.hpp
  include/header.hpp
  include/instrument.hpp
  include/level_book.hpp
  include/refdata.hpp
  include/shmem_sink.hpp
  include/sides.hpp
  include/sink.hpp
  include/timestamp.hpp
  include/trades.hpp
  include/types.hpp
  src/book.cpp
  src/config.cpp
  src/decimal.cpp
  src/engine.cpp
  src/header.cpp
  src/instrument.cpp
  src/level_book.cpp
  src/metrics.cpp
  src/refdata.cpp
  src/requests.cpp
  src/session.cpp
  src/shmem_sink.cpp
  src/sides.cpp
  src/trades.cpp
)
add_dependencies(kdr generate_all)

include_directories(${CMAKE_SOURCE_DIR}/parquet/include)
add_library(kdr_parquet
  parquet/include//book_sink.hpp
  parquet/include//io.hpp
  parquet/include//pairs_sink.hpp
  parquet/include//trade_sink.hpp
  parquet/include/assets_sink.hpp
  parquet/src/assets_sink.cpp
  parquet/src/book_sink.cpp
  parquet/src/pairs_sink.cpp
  parquet/src/trade_sink.cpp
)

target_link_libraries(kdr
  PUBLIC
  arrow::arrow
  boost::boost
  date::date
  openssl::openssl
  simdjson::simdjson
)

target_link_libraries(kdr_parquet
  PUBLIC
  arrow::arrow
  kdr
)

add_executable(kdr_record kdr_record/kdr_record.cpp)
target_link_libraries(kdr_record kdr_parquet)

add_executable(parse_instrument_snapshot test/parse_instrument_snapshot.cpp)
target_link_libraries(parse_instrument_snapshot kdr)

add_executable(check_book test/check_book.cpp)
target_link_libraries(check_book kdr)

##
# Unit tests
#
enable_testing()

add_executable(tests
  test/unit/asset_test.cpp
  test/unit/decimal_test.cpp
  test/unit/level_book_test.cpp
  test/unit/parquet_test.cpp
  test/unit/test_main.cpp
)
target_link_libraries(tests kdr doctest::doctest)

add_test(NAME unit_test COMMAND tests)
