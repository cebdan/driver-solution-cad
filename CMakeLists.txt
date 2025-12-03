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
    src/Core/Solution.cpp
    src/Core/SolutionFactory.cpp
    src/Core/RegisterSolutions.cpp
    src/Solutions/PointSolution.cpp
    src/Solutions/LineSolution.cpp
    src/Solutions/CoordinateSystemSolution.cpp
    src/Solutions/CircleSolution.cpp
    src/Solutions/SketchSolution.cpp
    src/Solutions/ConstraintSolution.cpp
    src/Solutions/ExtrudeSolution.cpp
    src/Solutions/RevolveSolution.cpp
    src/Solutions/FilletSolution.cpp
)

target_include_directories(CADCore PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# Solutions library (part of CADCore)
target_include_directories(CADCore PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include/Solutions
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
        
        add_executable(test_Solution tests/test_Solution.cpp)
        target_link_libraries(test_Solution CADCore GTest::GTest GTest::Main)
        add_test(NAME SolutionTest COMMAND test_Solution)
        
        add_executable(test_SolutionFactory tests/test_SolutionFactory.cpp)
        target_link_libraries(test_SolutionFactory CADCore GTest::GTest GTest::Main)
        add_test(NAME SolutionFactoryTest COMMAND test_SolutionFactory)
        
        add_executable(test_Kernel_Integration tests/test_Kernel_Integration.cpp)
        target_link_libraries(test_Kernel_Integration CADCore GTest::GTest GTest::Main)
        add_test(NAME KernelIntegrationTest COMMAND test_Kernel_Integration)
        
        add_executable(test_PointSolution tests/test_PointSolution.cpp)
        target_link_libraries(test_PointSolution CADCore GTest::GTest GTest::Main)
        add_test(NAME PointSolutionTest COMMAND test_PointSolution)
        
        add_executable(test_LineSolution tests/test_LineSolution.cpp)
        target_link_libraries(test_LineSolution CADCore GTest::GTest GTest::Main)
        add_test(NAME LineSolutionTest COMMAND test_LineSolution)
        
        add_executable(test_CoordinateSystemSolution tests/test_CoordinateSystemSolution.cpp)
        target_link_libraries(test_CoordinateSystemSolution CADCore GTest::GTest GTest::Main)
        add_test(NAME CoordinateSystemSolutionTest COMMAND test_CoordinateSystemSolution)
        
        add_executable(test_CircleSolution tests/test_CircleSolution.cpp)
        target_link_libraries(test_CircleSolution CADCore GTest::GTest GTest::Main)
        add_test(NAME CircleSolutionTest COMMAND test_CircleSolution)
        
        add_executable(test_SketchSolution tests/test_SketchSolution.cpp)
        target_link_libraries(test_SketchSolution CADCore GTest::GTest GTest::Main)
        add_test(NAME SketchSolutionTest COMMAND test_SketchSolution)
        
        add_executable(test_ConstraintSolution tests/test_ConstraintSolution.cpp)
        target_link_libraries(test_ConstraintSolution CADCore GTest::GTest GTest::Main)
        add_test(NAME ConstraintSolutionTest COMMAND test_ConstraintSolution)
        
        add_executable(test_ExtrudeSolution tests/test_ExtrudeSolution.cpp)
        target_link_libraries(test_ExtrudeSolution CADCore GTest::GTest GTest::Main)
        add_test(NAME ExtrudeSolutionTest COMMAND test_ExtrudeSolution)
        
        add_executable(test_RevolveSolution tests/test_RevolveSolution.cpp)
        target_link_libraries(test_RevolveSolution CADCore GTest::GTest GTest::Main)
        add_test(NAME RevolveSolutionTest COMMAND test_RevolveSolution)
        
        add_executable(test_FilletSolution tests/test_FilletSolution.cpp)
        target_link_libraries(test_FilletSolution CADCore GTest::GTest GTest::Main)
        add_test(NAME FilletSolutionTest COMMAND test_FilletSolution)
    else()
        message(WARNING "Google Test not found - tests will not be built")
        message(STATUS "Install with: brew install googletest (macOS) or apt-get install libgtest-dev (Linux)")
    endif()
endif()

# Install rules (placeholder)
# install(TARGETS ${PROJECT_NAME} DESTINATION bin)

