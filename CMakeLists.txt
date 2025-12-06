cmake_minimum_required(VERSION 3.20)
project(driver-solution-cad VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find xtd package
set(xtd_POSSIBLE_PATHS
    "/usr/local/cmake"
    "/usr/local/lib/cmake/xtd"
    "/opt/local/lib/cmake/xtd"
    "/usr/lib/cmake/xtd"
)

foreach(PATH ${xtd_POSSIBLE_PATHS})
    if(EXISTS "${PATH}/xtdConfig.cmake")
        set(xtd_DIR "${PATH}")
        message(STATUS "Found xtd in: ${PATH}")
        break()
    endif()
endforeach()

find_package(xtd QUIET)

if(NOT xtd_FOUND)
    message(FATAL_ERROR "xtd not found! Please install xtd first:\n  cd /Users/user/Documents/xtd/build\n  sudo cmake --install .")
endif()

message(STATUS "Found xtd: ${xtd_DIR}")

# Include directories
include_directories(${CMAKE_SOURCE_DIR}/include)

# Source files
set(SOURCES
    src/ConstructionHistory.cpp
    src/Node.cpp
    src/Solution.cpp
    src/SolutionDocument.cpp
    src/Document2D.cpp
    src/MainWindow.cpp
    src/Value.cpp
    src/CS.cpp
    src/2D_point.cpp
    src/3D_point.cpp
    src/TerminalWindow.cpp
    src/XTD.cpp
    src/OpenGLRenderer.cpp
    src/DataExchange.cpp
)

# Header files
set(HEADERS
    include/ConstructionHistory.h
    include/Node.h
    include/Solution.h
    include/SolutionDocument.h
    include/Document2D.h
    include/MainWindow.h
    include/Value.h
    include/CS.h
    include/2D_point.h
    include/3D_point.h
    include/TerminalWindow.h
    include/XTD.h
    include/OpenGLRenderer.h
    include/DataExchange.h
)

# Create library (without MainWindow for now to avoid compilation errors)
set(SOURCES_WITHOUT_MAINWINDOW ${SOURCES})
list(REMOVE_ITEM SOURCES_WITHOUT_MAINWINDOW src/MainWindow.cpp)
set(HEADERS_WITHOUT_MAINWINDOW ${HEADERS})
list(REMOVE_ITEM HEADERS_WITHOUT_MAINWINDOW include/MainWindow.h)

add_library(driver_solution_cad STATIC ${SOURCES_WITHOUT_MAINWINDOW} ${HEADERS_WITHOUT_MAINWINDOW})

# Link xtd if found
if(xtd_FOUND)
    if(TARGET xtd::xtd)
        target_link_libraries(driver_solution_cad PUBLIC xtd::xtd)
    elseif(TARGET xtd)
        target_link_libraries(driver_solution_cad PUBLIC xtd)
    elseif(DEFINED xtd_INCLUDE_DIRS AND DEFINED xtd_LIBRARIES)
        target_include_directories(driver_solution_cad PUBLIC ${xtd_INCLUDE_DIRS})
        target_link_directories(driver_solution_cad PUBLIC ${xtd_LIBRARIES_DIRS})
        target_link_libraries(driver_solution_cad PUBLIC ${xtd_LIBRARIES})
    endif()
endif()

# Demo terminal application
add_executable(demo_terminal examples/demo_terminal.cpp)
target_link_libraries(demo_terminal PRIVATE driver_solution_cad)

# Demo main menu application
add_executable(demo_main_menu examples/demo_main_menu.cpp)
target_link_libraries(demo_main_menu PRIVATE driver_solution_cad)

# Simple GUI application (requires xtd)
if(xtd_FOUND)
    add_executable(simple_gui examples/simple_gui_working.cpp)
    if(TARGET xtd::xtd)
        target_link_libraries(simple_gui PRIVATE xtd::xtd)
    elseif(TARGET xtd)
        target_link_libraries(simple_gui PRIVATE xtd)
    elseif(DEFINED xtd_INCLUDE_DIRS)
        target_include_directories(simple_gui PRIVATE ${xtd_INCLUDE_DIRS})
        if(DEFINED xtd_LIBRARIES_DIRS)
            target_link_directories(simple_gui PRIVATE ${xtd_LIBRARIES_DIRS})
        endif()
        if(DEFINED xtd_LIBRARIES)
            target_link_libraries(simple_gui PRIVATE ${xtd_LIBRARIES})
        endif()
    endif()
    if(APPLE)
        set_target_properties(simple_gui PROPERTIES
            MACOSX_BUNDLE TRUE
            MACOSX_BUNDLE_GUI_IDENTIFIER "com.cadsystem.simplegui"
        )
    endif()
endif()

# Installation (optional)
install(TARGETS driver_solution_cad
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
)

install(FILES ${HEADERS}
    DESTINATION include
)
