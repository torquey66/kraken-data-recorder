cmake_minimum_required(VERSION 3.29.2)

project(KrakpotProject)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_EXTENSIONS ON)

set(CMAKE_OSX_DEPLOYMENT_TARGET "10.14" CACHE STRING "Minimum OS X deployment version")

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
  ${CMAKE_SOURCE_DIR}/codegen/asset.json
)

set(ENUM_FILES
  ${CMAKE_SOURCE_DIR}/codegen/asset_status.json
)

set(GENERATED_FILES
  ${CMAKE_SOURCE_DIR}/generated/asset.hpp
  ${CMAKE_SOURCE_DIR}/generated/asset.cpp
  ${CMAKE_SOURCE_DIR}/generated/asset_status.hpp
  ${CMAKE_SOURCE_DIR}/generated/asset_status.cpp
)

foreach(model_file ${MODEL_FILES})
    get_filename_component(filename ${model_file} NAME_WE)
    set(header_file ${CMAKE_SOURCE_DIR}/generated/${filename}.hpp)
    set(code_file ${CMAKE_SOURCE_DIR}/generated/${filename}.cpp)
    # define template files and DEPEND on them below
    add_custom_command(
        OUTPUT ${header_file} ${code_file}
        COMMAND jinja2 ${CMAKE_SOURCE_DIR}/codegen/model_header.j2 ${model_file} | clang-format > ${header_file}
        COMMAND jinja2 ${CMAKE_SOURCE_DIR}/codegen/model_code.j2   ${model_file} | clang-format > ${code_file}
        DEPENDS ${model_file}
        COMMENT "Generating ${header_file} from ${model_file}"
        COMMENT "Generating ${code_file} from ${model_file}"
    )
endforeach()

foreach(enum_file ${ENUM_FILES})
    get_filename_component(filename ${enum_file} NAME_WE)
    set(header_file ${CMAKE_SOURCE_DIR}/generated/${filename}.hpp)
    set(code_file ${CMAKE_SOURCE_DIR}/generated/${filename}.cpp)
    # define template files and DEPEND on them below
    add_custom_command(
        OUTPUT ${header_file} ${code_file}
        COMMAND jinja2 ${CMAKE_SOURCE_DIR}/codegen/enum_header.j2 ${enum_file} | clang-format > ${header_file}
        COMMAND jinja2 ${CMAKE_SOURCE_DIR}/codegen/enum_code.j2   ${enum_file} | clang-format > ${code_file}
        DEPENDS ${enum_file}
        COMMENT "Generating ${header_file} from ${enum_file}"
        COMMENT "Generating ${code_file} from ${enum_file}"
    )
endforeach()

add_custom_target(generate_all
    ALL
    DEPENDS ${GENERATED_FILES}
)

include_directories(${CMAKE_SOURCE_DIR}/genreated)
add_library(krakpot_generated
  ${GENERATED_FILES}
)
add_dependencies(krakpot_generated generate_all)


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

target_link_libraries(krakpot_generated
  PUBLIC
  boost::boost
  simdjson::simdjson
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

add_executable(parse_instrument_snapshot test/parse_instrument_snapshot.cpp)
target_link_libraries(parse_instrument_snapshot krakpot_base)

add_executable(check_book test/check_book.cpp)
target_link_libraries(check_book krakpot_base)

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
target_link_libraries(tests krakpot_base doctest::doctest)

add_test(NAME unit_test COMMAND tests)
