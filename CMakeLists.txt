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

# GUI Application - Solution Manager
find_package(SDL2 QUIET)
if(NOT SDL2_FOUND)
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
    
    set(IMGUI_DIR "${CMAKE_SOURCE_DIR}/third_party/imgui" CACHE PATH "ImGui directory")
    set(IMSTUDIO_DIR "${CMAKE_SOURCE_DIR}/third_party/ImStudio" CACHE PATH "ImStudio directory")
    
    if(EXISTS "${IMGUI_DIR}/imgui.h")
        message(STATUS "Found ImGui in: ${IMGUI_DIR}")
        
        # ImStudio library
        if(EXISTS "${IMSTUDIO_DIR}/src/sources/ims_gui.h")
            message(STATUS "Found ImStudio in: ${IMSTUDIO_DIR}")
            
            # ImStudio sources
            file(GLOB_RECURSE IMSTUDIO_SOURCES 
                "${IMSTUDIO_DIR}/src/sources/*.cpp"
                "${IMSTUDIO_DIR}/src/utils/*.cpp"
            )
            file(GLOB_RECURSE IMSTUDIO_HEADERS 
                "${IMSTUDIO_DIR}/src/sources/*.h"
                "${IMSTUDIO_DIR}/src/utils/*.h"
            )
            
            # ImStudio uses its own imgui, but we'll use ours
            # We need imgui_stdlib for ImStudio
            add_library(imstudio STATIC
                ${IMSTUDIO_SOURCES}
                ${IMSTUDIO_HEADERS}
                ${IMSTUDIO_DIR}/src/main_window.cpp
            )
            
            target_include_directories(imstudio PUBLIC
                ${IMSTUDIO_DIR}/src/sources
                ${IMSTUDIO_DIR}/src/utils
                ${IMGUI_DIR}
                ${IMGUI_DIR}/misc/cpp  # for imgui_stdlib.h
            )
            
            # ImStudio needs fmt library (included in third-party)
            if(EXISTS "${IMSTUDIO_DIR}/src/third-party/fmt/include")
                target_include_directories(imstudio PUBLIC
                    ${IMSTUDIO_DIR}/src/third-party/fmt/include
                )
                # Add fmt source files
                target_sources(imstudio PRIVATE
                    ${IMSTUDIO_DIR}/src/third-party/fmt/src/format.cc
                    ${IMSTUDIO_DIR}/src/third-party/fmt/src/os.cc
                )
            endif()
            
            set(IMSTUDIO_FOUND TRUE)
        else()
            message(WARNING "ImStudio not found, UI Editor will use basic version")
            set(IMSTUDIO_FOUND FALSE)
        endif()
        
        # Solution Manager GUI application
        add_executable(solution_manager 
            examples/solution_manager.cpp
            ${IMGUI_DIR}/imgui.cpp
            ${IMGUI_DIR}/imgui_demo.cpp
            ${IMGUI_DIR}/imgui_draw.cpp
            ${IMGUI_DIR}/imgui_tables.cpp
            ${IMGUI_DIR}/imgui_widgets.cpp
            ${IMGUI_DIR}/backends/imgui_impl_sdl2.cpp
            ${IMGUI_DIR}/backends/imgui_impl_opengl3.cpp
            ${IMGUI_DIR}/misc/cpp/imgui_stdlib.cpp
        )
        
        target_include_directories(solution_manager PRIVATE
            ${CMAKE_SOURCE_DIR}/include
            ${IMGUI_DIR}
            ${IMGUI_DIR}/backends
            ${IMGUI_DIR}/misc/cpp  # for imgui_stdlib.h
            ${SDL2_INCLUDE_DIR}
        )
        
        if(IMSTUDIO_FOUND)
            target_include_directories(solution_manager PRIVATE
                ${IMSTUDIO_DIR}/src/sources
                ${IMSTUDIO_DIR}/src/utils
            )
            target_link_libraries(solution_manager PRIVATE imstudio)
            target_compile_definitions(solution_manager PRIVATE IMSTUDIO_AVAILABLE)
        endif()
        
        target_link_libraries(solution_manager PRIVATE cad_core)
        
        # Find SDL2 properly
        find_program(PKG_CONFIG_EXECUTABLE pkg-config)
        if(PKG_CONFIG_EXECUTABLE)
            execute_process(
                COMMAND ${PKG_CONFIG_EXECUTABLE} --libs sdl2
                OUTPUT_VARIABLE SDL2_LINK_LIBS
                OUTPUT_STRIP_TRAILING_WHITESPACE
            )
        endif()
        
        if(SDL2_LINK_LIBS)
            target_link_libraries(solution_manager PRIVATE ${SDL2_LINK_LIBS})
        elseif(TARGET SDL2::SDL2)
            target_link_libraries(solution_manager PRIVATE SDL2::SDL2)
        elseif(SDL2_LIBRARY)
            target_link_libraries(solution_manager PRIVATE ${SDL2_LIBRARY})
        else()
            find_library(SDL2_LIB SDL2
                PATHS
                /usr/local/lib
                /opt/homebrew/lib
                /usr/lib
            )
            if(SDL2_LIB)
                target_link_libraries(solution_manager PRIVATE ${SDL2_LIB})
            else()
                target_link_libraries(solution_manager PRIVATE SDL2)
            endif()
        endif()
        
        target_link_libraries(solution_manager PRIVATE ${CMAKE_DL_LIBS})
        
        # OpenGL
        if(APPLE)
            find_library(OPENGL_LIBRARY OpenGL
                PATHS /System/Library/Frameworks
            )
            target_link_libraries(solution_manager PRIVATE ${OPENGL_LIBRARY})
        else()
            find_package(OpenGL REQUIRED)
            target_link_libraries(solution_manager PRIVATE OpenGL::GL)
        endif()
        
        # macOS app bundle
        if(APPLE)
            set_target_properties(solution_manager PROPERTIES
                MACOSX_BUNDLE TRUE
                MACOSX_BUNDLE_BUNDLE_NAME "Solution Manager"
                MACOSX_BUNDLE_GUI_IDENTIFIER "com.driver-solution-cad.solution-manager"
            )
        endif()
    else()
        message(WARNING "ImGui not found, GUI application will not be built")
    endif()
else()
    message(WARNING "SDL2 not found, GUI application will not be built")
endif()
