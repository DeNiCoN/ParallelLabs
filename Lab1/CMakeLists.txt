project(Lab1)
cmake_minimum_required(VERSION 3.22.2)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

file(GLOB lab1_SRC
  "src/*.hpp"
  "src/*.cpp"
  )

add_executable(lab1
  ${lab1_SRC}
  )

# Using conan for dependency management
# Run conan install .. --build missing
# from build directory before configuration
find_package(fmt)
find_package(Boost REQUIRED COMPONENTS program_options system)
find_package(nlohmann_json)

target_link_libraries(lab1
  Boost::program_options
  Boost::system
  fmt::fmt
  Lib::Lib
  nlohmann_json::nlohmann_json
  )

target_compile_definitions(lab1
  PRIVATE FMT_HEADER_ONLY
  )
