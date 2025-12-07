cmake_minimum_required(VERSION 3.16)
project(driver-solution-cad VERSION 2.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Core library
add_library(cad_core STATIC
    src/Solution.cpp
    src/Value.cpp
    src/CS.cpp
    src/Point.cpp
    src/Line.cpp
)

target_include_directories(cad_core PUBLIC ${CMAKE_SOURCE_DIR}/include)

# Tests
add_executable(test_core tests/test_core.cpp)
target_link_libraries(test_core PRIVATE cad_core)

# Example
add_executable(example_chain examples/example_chain.cpp)
target_link_libraries(example_chain PRIVATE cad_core)
