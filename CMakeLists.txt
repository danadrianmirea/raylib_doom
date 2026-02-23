cmake_minimum_required(VERSION 3.15)
project(minimalistic_raylib_template_vscode LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Optionally pass RAYLIB_PATH to locate a local raylib installation
set(RAYLIB_PATH "c:/raylib/raylib" CACHE PATH "c:/raylib/raylib")

add_subdirectory(src)
