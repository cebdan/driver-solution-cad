cmake_minimum_required(VERSION 3.15)
project(driver-solution-cad VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Build options
option(BUILD_TESTS "Build unit tests" ON)
option(BUILD_UI "Build UI components" OFF)

# Output directories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# Include directories
include_directories(${CMAKE_SOURCE_DIR}/include)

# Source files (to be added as project grows)
file(GLOB_RECURSE SOURCES "src/*.cpp" "src/*.h")
file(GLOB_RECURSE HEADERS "include/*.h" "include/*.hpp")

# Main executable (placeholder)
# add_executable(${PROJECT_NAME} ${SOURCES})

# Tests
if(BUILD_TESTS)
    enable_testing()
    # Add Catch2 or other testing framework here
endif()

# Install rules (placeholder)
# install(TARGETS ${PROJECT_NAME} DESTINATION bin)

