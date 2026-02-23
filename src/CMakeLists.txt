cmake_minimum_required(VERSION 3.15)

file(GLOB_RECURSE SRC_FILES CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp ${CMAKE_CURRENT_SOURCE_DIR}/*.c)

add_executable(minimalistic_raylib_template_vscode ${SRC_FILES})

# Try to find an installed raylib package first
find_package(raylib QUIET)

if (raylib_FOUND)
    target_link_libraries(minimalistic_raylib_template_vscode PRIVATE raylib)
else()
    if (RAYLIB_PATH)
        message(STATUS "Using RAYLIB_PATH=${RAYLIB_PATH}")
        # Common layout: include and libs in RAYLIB_PATH/src
        set(RAYLIB_INCLUDE_DIR "${RAYLIB_PATH}/src")
        if (EXISTS "${RAYLIB_PATH}/src/libraylib.a")
            set(RAYLIB_LIB "${RAYLIB_PATH}/src/libraylib.a")
        elseif (EXISTS "${RAYLIB_PATH}/src/raylib.lib")
            set(RAYLIB_LIB "${RAYLIB_PATH}/src/raylib.lib")
        elseif (EXISTS "${RAYLIB_PATH}/lib/libraylib.a")
            set(RAYLIB_LIB "${RAYLIB_PATH}/lib/libraylib.a")
        elseif (EXISTS "${RAYLIB_PATH}/lib/raylib.lib")
            set(RAYLIB_LIB "${RAYLIB_PATH}/lib/raylib.lib")
        endif()

        if (EXISTS "${RAYLIB_INCLUDE_DIR}")
            target_include_directories(minimalistic_raylib_template_vscode PRIVATE "${RAYLIB_INCLUDE_DIR}")
        endif()

        if (RAYLIB_LIB)
            target_link_libraries(minimalistic_raylib_template_vscode PRIVATE "${RAYLIB_LIB}")
        else()
            message(WARNING "No libraylib library found under ${RAYLIB_PATH}/lib - you may need to adjust RAYLIB_PATH or install raylib.")
        endif()

        # Platform helper libs (useful for Windows builds with MinGW)
        if (WIN32)
            target_link_libraries(minimalistic_raylib_template_vscode PRIVATE opengl32 gdi32 winmm)
        endif()
    else()
        message(WARNING "raylib not found. Set -DRAYLIB_PATH=/path/to/raylib or install raylib CMake package.")
    endif()
endif()
