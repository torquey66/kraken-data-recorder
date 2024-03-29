cmake_minimum_required(VERSION 3.0)
project(KrakpotProject)

add_compile_options(-fsanitize=address)
add_link_options(-fsanitize=address)

# Boost
find_package(Boost REQUIRED COMPONENTS fiber log system)
if(Boost_FOUND)
    include_directories(${Boost_INCLUDE_DIRS})
endif()

# OpenSSL
find_package(OpenSSL REQUIRED)
if(OPENSSL_FOUND)
    include_directories(${OPENSSL_INCLUDE_DIR})
endif()

# simdjson
find_package(simdjson REQUIRED)
if(simdjson_FOUND)
  include_directories(${simdjson_INCLUDE_DIRS_DEBUG})
endif()

add_executable(Krakpot
  book.cpp
  cert.cpp
  main.cpp
  pair.cpp
  processor.cpp
  record.cpp
  session.cpp
)

target_link_libraries(Krakpot
  Boost::fiber
  Boost::log
  Boost::system
  ${OPENSSL_LIBRARIES}
  ${simdjson_LIBRARIES}
)

add_executable(parse_instrument_snapshot
  pair.cpp
  test/parse_instrument_snapshot.cpp
)

target_link_libraries(parse_instrument_snapshot
  ${simdjson_LIBRARIES}
)

