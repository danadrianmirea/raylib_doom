cmake_minimum_required(VERSION 3.15)
project(raylib_doom LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

# Optionally pass RAYLIB_PATH to locate a local raylib installation
set(RAYLIB_PATH "c:/raylib/raylib" CACHE PATH "c:/raylib/raylib")

add_subdirectory(src)
