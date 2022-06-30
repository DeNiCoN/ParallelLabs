project(Parallel)
cmake_minimum_required(VERSION 3.22.2)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Using conan for dependency management
# Run conan install .. --build missing
# from build directory before configuration
set(CMAKE_MODULE_PATH ${CMAKE_BINARY_DIR})

add_subdirectory(Lib)
add_subdirectory(Lab1)
add_subdirectory(Lab2)
add_subdirectory(Lab3)
add_subdirectory(Lab4)
add_subdirectory(Lab5)
add_subdirectory(Exam)

add_subdirectory(tests)
