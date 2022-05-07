set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

file(GLOB lib_SRC
  "*.hpp"
  "*.cpp"
  )

add_library(Lib INTERFACE
  )

target_include_directories(Lib INTERFACE .)

add_library(Lib::Lib ALIAS Lib)
