cmake_minimum_required(VERSION 3.15)

file(GLOB_RECURSE SRC_FILES CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp ${CMAKE_CURRENT_SOURCE_DIR}/*.c)

# Exclude platform-specific doomgeneric backends when building on incompatible platforms
if (WIN32)
    foreach(_f IN LISTS SRC_FILES)
        if (_f MATCHES "/doomgeneric/doomgeneric_sdl.c$" OR _f MATCHES "/doomgeneric/doomgeneric_allegro.c$" OR _f MATCHES "/doomgeneric/doomgeneric_emscripten.c$" OR _f MATCHES "/doomgeneric/doomgeneric_linuxvt.c$" OR _f MATCHES "/doomgeneric/doomgeneric_soso.c$" OR _f MATCHES "/doomgeneric/doomgeneric_sosox.c$")
            list(REMOVE_ITEM SRC_FILES "${_f}")
        endif()
    endforeach()
endif()

add_executable(raylib_doom ${SRC_FILES})

# Try to find an installed raylib package first
find_package(raylib QUIET)

if (raylib_FOUND)
    target_link_libraries(raylib_doom PRIVATE raylib)
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
            target_include_directories(raylib_doom PRIVATE "${RAYLIB_INCLUDE_DIR}")
        endif()

        if (RAYLIB_LIB)
            target_link_libraries(raylib_doom PRIVATE "${RAYLIB_LIB}")
        else()
            message(WARNING "No libraylib library found under ${RAYLIB_PATH}/lib - you may need to adjust RAYLIB_PATH or install raylib.")
        endif()

        # Platform helper libs (useful for Windows builds with MinGW)
        if (WIN32)
            target_link_libraries(raylib_doom PRIVATE opengl32 gdi32 winmm)
        endif()
    else()
        message(WARNING "raylib not found. Set -DRAYLIB_PATH=/path/to/raylib or install raylib CMake package.")
    endif()
endif()
