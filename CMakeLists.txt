# Works with 3.15 and tested through 4.0
cmake_minimum_required(VERSION 3.15...4.0)

# Project name and a few useful settings. Other commands can pick up the results
project(
  Breakout
  VERSION 0.1
  DESCRIPTION "A simple Breakout game"
  LANGUAGES C CXX)

# FetchContent added in CMake 3.11, downloads during the configure step
# FetchContent_MakeAvailable was added in CMake 3.14; simpler usage
include(FetchContent)

# GLFW
FetchContent_Declare(
    glfw
    GIT_REPOSITORY https://github.com/glfw/glfw
    GIT_TAG        3.3.8  # or latest stable
)
FetchContent_MakeAvailable(glfw)

# GLM
FetchContent_Declare(
    glm
    GIT_REPOSITORY https://github.com/g-truc/glm
    GIT_TAG        1.0.1  # or latest stable
)
FetchContent_MakeAvailable(glm)

# miniaudio
FetchContent_Declare(
    miniaudio
    GIT_REPOSITORY https://github.com/mackron/miniaudio.git
    GIT_TAG        0.11.22  # or latest stable
)
FetchContent_MakeAvailable(miniaudio)

# Freetype
FetchContent_Declare(
    freetype
    URL https://download.savannah.gnu.org/releases/freetype/freetype-2.13.3.tar.gz
)
FetchContent_MakeAvailable(freetype)

add_subdirectory(apps)

add_subdirectory(src)
