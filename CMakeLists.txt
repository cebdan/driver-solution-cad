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
    src/StartWindow.cpp
    src/Value.cpp
    src/CS.cpp
    src/2D_point.cpp
    src/3D_point.cpp
    src/TerminalWindow.cpp
    src/XTD.cpp
    src/OpenGLRenderer.cpp
    src/DataExchange.cpp
    src/close_app.cpp
)

# Header files
set(HEADERS
    include/ConstructionHistory.h
    include/Node.h
    include/Solution.h
    include/SolutionDocument.h
    include/Document2D.h
    include/StartWindow.h
    include/Value.h
    include/CS.h
    include/2D_point.h
    include/3D_point.h
    include/TerminalWindow.h
    include/XTD.h
    include/OpenGLRenderer.h
    include/DataExchange.h
    include/close_app.h
)

# Create library (without StartWindow for now to avoid compilation errors)
set(SOURCES_WITHOUT_STARTWINDOW ${SOURCES})
list(REMOVE_ITEM SOURCES_WITHOUT_STARTWINDOW src/StartWindow.cpp)
set(HEADERS_WITHOUT_STARTWINDOW ${HEADERS})
list(REMOVE_ITEM HEADERS_WITHOUT_STARTWINDOW include/StartWindow.h)

add_library(driver_solution_cad STATIC ${SOURCES_WITHOUT_STARTWINDOW} ${HEADERS_WITHOUT_STARTWINDOW})

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
    if(APPLE)
        # On macOS, compile with Objective-C++ support for window utilities
        add_executable(simple_gui examples/simple_gui_working.cpp src/close_app.cpp src/macos_window_utils.mm)
        set_source_files_properties(src/macos_window_utils.mm PROPERTIES COMPILE_FLAGS "-x objective-c++")
    else()
        add_executable(simple_gui examples/simple_gui_working.cpp src/close_app.cpp)
    endif()
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

# ImGui + SDL2 GUI application (alternative to xtd)
find_package(SDL2 QUIET)
if(NOT SDL2_FOUND)
    # Try to find SDL2 using pkg-config or manual paths
    find_path(SDL2_INCLUDE_DIR SDL.h
        PATHS
        /usr/local/include/SDL2
        /usr/include/SDL2
        /opt/homebrew/include/SDL2
    )
    find_library(SDL2_LIBRARY SDL2
        PATHS
        /usr/local/lib
        /usr/lib
        /opt/homebrew/lib
    )
    if(SDL2_INCLUDE_DIR AND SDL2_LIBRARY)
        set(SDL2_FOUND TRUE)
    endif()
endif()

if(SDL2_FOUND)
    message(STATUS "Found SDL2")
    
    # ImGui is header-only, we'll include it directly
    # Download or include ImGui sources
    set(IMGUI_DIR "${CMAKE_SOURCE_DIR}/third_party/imgui" CACHE PATH "ImGui directory")
    
    if(EXISTS "${IMGUI_DIR}/imgui.h")
        message(STATUS "Found ImGui in: ${IMGUI_DIR}")
        
        # ImGui + SDL2 application
        if(APPLE)
            add_executable(simple_gui_imgui 
                examples/simple_gui_imgui.cpp 
                src/imgui_macos_utils.mm
                ${IMGUI_DIR}/imgui.cpp
                ${IMGUI_DIR}/imgui_demo.cpp
                ${IMGUI_DIR}/imgui_draw.cpp
                ${IMGUI_DIR}/imgui_tables.cpp
                ${IMGUI_DIR}/imgui_widgets.cpp
                ${IMGUI_DIR}/backends/imgui_impl_sdl2.cpp
                ${IMGUI_DIR}/backends/imgui_impl_opengl3.cpp
            )
            set_source_files_properties(src/imgui_macos_utils.mm PROPERTIES COMPILE_FLAGS "-x objective-c++")
        else()
            add_executable(simple_gui_imgui 
                examples/simple_gui_imgui.cpp 
                src/close_app.cpp
                src/imgui_macos_utils.mm
                ${IMGUI_DIR}/imgui.cpp
                ${IMGUI_DIR}/imgui_demo.cpp
                ${IMGUI_DIR}/imgui_draw.cpp
                ${IMGUI_DIR}/imgui_tables.cpp
                ${IMGUI_DIR}/imgui_widgets.cpp
                ${IMGUI_DIR}/backends/imgui_impl_sdl2.cpp
                ${IMGUI_DIR}/backends/imgui_impl_opengl3.cpp
            )
        endif()
        
        target_include_directories(simple_gui_imgui PRIVATE 
            ${SDL2_INCLUDE_DIR}
            ${IMGUI_DIR}
            ${IMGUI_DIR}/backends
        )
        
        # Find SDL2 properly using pkg-config or manual
        find_program(PKG_CONFIG_EXECUTABLE pkg-config)
        if(PKG_CONFIG_EXECUTABLE)
            execute_process(
                COMMAND ${PKG_CONFIG_EXECUTABLE} --libs sdl2
                OUTPUT_VARIABLE SDL2_LINK_LIBS
                OUTPUT_STRIP_TRAILING_WHITESPACE
            )
        endif()
        
        if(SDL2_LINK_LIBS)
            target_link_libraries(simple_gui_imgui PRIVATE ${SDL2_LINK_LIBS})
        elseif(TARGET SDL2::SDL2)
            target_link_libraries(simple_gui_imgui PRIVATE SDL2::SDL2)
        elseif(SDL2_LIBRARY)
            target_link_libraries(simple_gui_imgui PRIVATE ${SDL2_LIBRARY})
        else()
            # Fallback: try common locations
            find_library(SDL2_LIB SDL2
                PATHS
                /usr/local/lib
                /opt/homebrew/lib
                /usr/lib
            )
            if(SDL2_LIB)
                target_link_libraries(simple_gui_imgui PRIVATE ${SDL2_LIB})
            else()
                target_link_libraries(simple_gui_imgui PRIVATE SDL2)
            endif()
        endif()
        
        target_link_libraries(simple_gui_imgui PRIVATE ${CMAKE_DL_LIBS})
        
        if(APPLE)
            target_link_libraries(simple_gui_imgui PRIVATE 
                "-framework OpenGL"
                "-framework Cocoa"
            )
            set_target_properties(simple_gui_imgui PROPERTIES
                MACOSX_BUNDLE TRUE
                MACOSX_BUNDLE_GUI_IDENTIFIER "com.cadsystem.simpleguiimgui"
            )
        else()
            find_package(OpenGL REQUIRED)
            target_link_libraries(simple_gui_imgui PRIVATE OpenGL::GL)
        endif()
        
    else()
        message(WARNING "ImGui not found in ${IMGUI_DIR}. Please download ImGui to third_party/imgui")
    endif()
else()
    message(STATUS "SDL2 not found. ImGui GUI application will not be built.")
endif()

# Installation (optional)
install(TARGETS driver_solution_cad
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
)

install(FILES ${HEADERS}
    DESTINATION include
)
