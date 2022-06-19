project(Lab5)
cmake_minimum_required(VERSION 3.22.2)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

file(GLOB lab5_SRC
  "src/*.hpp"
  "src/*.cpp"
  )

add_executable(client
  "src/client.cpp"
  "src/common.hpp"
  )

add_executable(server
  "src/server.cpp"
  "src/common.hpp"
  )

# Using conan for dependency management
# Run conan install .. --build missing
# from build directory before configuration
find_package(fmt)
find_package(Boost REQUIRED COMPONENTS program_options system serialization)
find_package(nlohmann_json)
find_package(TBB REQUIRED)

target_link_libraries(client
  Boost::program_options
  Boost::system
  Boost::serialization
  fmt::fmt
  Lib::Lib
  TBB::tbb
  nlohmann_json::nlohmann_json
  )

target_link_libraries(server
  Boost::program_options
  Boost::system
  Boost::serialization
  fmt::fmt
  Lib::Lib
  TBB::tbb
  nlohmann_json::nlohmann_json
  )
