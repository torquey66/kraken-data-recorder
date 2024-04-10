cmake_minimum_required(VERSION 3.16)
project(KrakpotProject)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_EXTENSIONS ON)

add_compile_options(-fsanitize=address)
add_link_options(-fsanitize=address)

add_compile_options(-Wall -Wextra -Wpedantic -Werror)

##
# External dependencies
#
find_package(Arrow REQUIRED)
if (Arrow_FOUND)
    include_directories(SYSTEM ${Arrow_INCLUDE_DIRS})
endif()

find_package(Boost REQUIRED)
if(Boost_FOUND)
    include_directories(SYSTEM ${Boost_INCLUDE_DIRS})
endif()

find_package(doctest REQUIRED)
if(doctest_FOUND)
  include_directories(SYSTEM ${doctest_INCLUDE_DIRS})
endif()

find_package(nlohmann_json)
if(nlohmann_json_FOUND)
  include_directories(SYSTEM ${nlohmann_json_INCLUDE_DIRS})
endif()

find_package(OpenSSL REQUIRED)
if(OPENSSL_FOUND)
    include_directories(SYSTEM ${OPENSSL_INCLUDE_DIR})
endif()

set(simdjson_DIR, ${CMAKE_CURRENT_BINARY_DIR})
find_package(simdjson REQUIRED)
if(simdjson_FOUND)
  include_directories(SYSTEM ${simdjson_INCLUDE_DIRS})
endif()

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

# get_cmake_property(_variableNames VARIABLES)
# list (SORT _variableNames)
# foreach (_variableName ${_variableNames})
#     message(STATUS "${_variableName}=${${_variableName}}")
# endforeach()

target_link_libraries(Krakpot
  Boost::fiber
  Boost::log
  Boost::system
  ${arrow_LIBRARIES_DEBUG}
  ${JSONCPP_LIBRARIES}
  ${OPENSSL_LIBRARIES}
  ${PARQUET_LIBRARIES}
  ${simdjson_LIBRARIES}
)

add_executable(parse_instrument_snapshot
  asset.cpp
  pair.cpp
  responses.cpp
  test/parse_instrument_snapshot.cpp
)

target_link_libraries(parse_instrument_snapshot
  ${simdjson_LIBRARIES}
)

add_executable(response_test
  asset.cpp
  pair.cpp
  responses.cpp
  test/unit/response_test.cpp
)

target_link_libraries(response_test
  ${doctest_LIBRARIES}
  ${simdjson_LIBRARIES}
)

