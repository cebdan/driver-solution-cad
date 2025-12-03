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

# Core library
add_library(CADCore STATIC
    src/Core/DependencyGraph.cpp
    src/Core/Kernel.cpp
)

target_include_directories(CADCore PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# Tests
if(BUILD_TESTS)
    enable_testing()
    
    # Find Google Test
    find_package(GTest QUIET)
    
    if(GTest_FOUND)
        message(STATUS "Google Test found - building tests")
        
        add_executable(test_DependencyGraph tests/test_DependencyGraph.cpp)
        target_link_libraries(test_DependencyGraph CADCore GTest::GTest GTest::Main)
        add_test(NAME DependencyGraphTest COMMAND test_DependencyGraph)
        
        add_executable(test_Kernel tests/test_Kernel.cpp)
        target_link_libraries(test_Kernel CADCore GTest::GTest GTest::Main)
        add_test(NAME KernelTest COMMAND test_Kernel)
    else()
        message(WARNING "Google Test not found - tests will not be built")
        message(STATUS "Install with: brew install googletest (macOS) or apt-get install libgtest-dev (Linux)")
    endif()
endif()

# Install rules (placeholder)
# install(TARGETS ${PROJECT_NAME} DESTINATION bin)

