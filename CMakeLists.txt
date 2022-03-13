cmake_minimum_required(VERSION 3.21)
project(cpp-2d-game-engine VERSION 0.1.0)

# include(warnings)

add_library(stb INTERFACE)
target_sources(stb INTERFACE "Libraries/stb_image/stb_image.cpp")
target_include_directories(stb INTERFACE "Libraries/stb_image")

add_subdirectory(Spiel/src)
# set_project_warnings(cuiui PUBLIC)
