# Works with 3.15 and tested through 4.0
cmake_minimum_required(VERSION 3.15...4.0)

# Project name and a few useful settings. Other commands can pick up the results
project(
  PongNet
  VERSION 0.1
  DESCRIPTION "A simple online Pong game"
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

# Asio 
FetchContent_Declare(
    asio
    GIT_REPOSITORY https://github.com/chriskohlhoff/asio
    GIT_TAG asio-1-28-0
)
FetchContent_MakeAvailable(asio)

# Define asio as an INTERFACE library
add_library(asio INTERFACE)
target_include_directories(asio INTERFACE ${asio_SOURCE_DIR}/asio/include)
target_compile_definitions(asio INTERFACE ASIO_STANDALONE)

# NetCommon
FetchContent_Declare(
    NetCommon
    GIT_REPOSITORY https://github.com/LT2H/NetCommon.git
    GIT_TAG main
)
FetchContent_MakeAvailable(NetCommon)

# ImGui
FetchContent_Declare(
    imgui
    GIT_REPOSITORY https://github.com/ocornut/imgui
    GIT_TAG        master
)
FetchContent_MakeAvailable(imgui)

# Create ImGui backend target
add_library(imgui_backend
    ${imgui_SOURCE_DIR}/imgui.cpp
    ${imgui_SOURCE_DIR}/imgui_draw.cpp
    ${imgui_SOURCE_DIR}/imgui_tables.cpp
    ${imgui_SOURCE_DIR}/imgui_widgets.cpp
    ${imgui_SOURCE_DIR}/imgui_demo.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_glfw.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_opengl3.cpp
)

target_include_directories(imgui_backend PUBLIC
    ${imgui_SOURCE_DIR}
    ${imgui_SOURCE_DIR}/backends
)

target_link_libraries(imgui_backend PUBLIC glfw OpenGL::GL)
find_package(OpenGL REQUIRED)

add_subdirectory(apps)

add_subdirectory(src)
