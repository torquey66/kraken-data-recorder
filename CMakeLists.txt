cmake_minimum_required(VERSION 3.0)
project(KrakpotProject)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)

# Set the path to Boost installation directory if not found automatically
#set(BOOST_ROOT /usr/local/opt/boost)
#set(BOOST_LIBRARYDIR ${BOOST_ROOT}/lib) # Path to Boost libraries

# Find Boost
find_package(Boost REQUIRED COMPONENTS fiber log system) # beast)
if(Boost_FOUND)
    include_directories(${Boost_INCLUDE_DIRS})
endif()

# Find OpenSSL
find_package(OpenSSL REQUIRED)
if(OPENSSL_FOUND)
    include_directories(${OPENSSL_INCLUDE_DIR})
endif()

find_package(nlohmann_json REQUIRED)
if(nlohmann_json_FOUND)
  include_directories(${nlohmann_json_INCLUDE_DIRS_DEBUG})
endif()

#include_directories(/usr/local/Cellar/nlohmann-json/3.11.3/include)

# Add executable
add_executable(Krakpot main.cpp book.cpp cert.cpp session.cpp)

# Link Boost and OpenSSL libraries
target_link_libraries(Krakpot Boost::fiber Boost::log Boost::system ${OPENSSL_LIBRARIES})
