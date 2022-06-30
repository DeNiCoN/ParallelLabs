project(Exam)
cmake_minimum_required(VERSION 3.22.2)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

file(GLOB exam_SRC
  "src/*.hpp"
  "src/*.cpp"
  )

add_executable(exam
  ${exam_SRC}
  )

# Using conan for dependency management
# Run conan install .. --build missing
# from build directory before configuration
find_package(fmt)
find_package(Boost REQUIRED COMPONENTS program_options system)
find_package(nlohmann_json)
find_package(TBB REQUIRED)

target_link_libraries(exam
  Boost::program_options
  Boost::system
  fmt::fmt
  Lib::Lib
  TBB::tbb
  nlohmann_json::nlohmann_json
  )

target_compile_definitions(exam
  PRIVATE FMT_HEADER_ONLY
  )
