cmake_minimum_required(VERSION 3.21)
project(PhysicsSandbox VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()

# ---------------------------------------------------------------------------
# Dependencies (downloaded at configure time with FetchContent, versions pinned)
# ---------------------------------------------------------------------------
include(FetchContent)

# One static executable: no DLLs/.so files to ship next to it.
set(BUILD_SHARED_LIBS OFF CACHE BOOL "Build dependencies as static libraries" FORCE)

# SFML 2.6.1 - window, rendering, input (audio/network are not needed).
set(SFML_BUILD_AUDIO OFF CACHE BOOL "" FORCE)
set(SFML_BUILD_NETWORK OFF CACHE BOOL "" FORCE)
FetchContent_Declare(SFML
  GIT_REPOSITORY https://github.com/SFML/SFML.git
  GIT_TAG        2.6.1
  GIT_SHALLOW    TRUE)

# Box2D 2.4.1 - the classic C++ API (NOT the v3 C API).
set(BOX2D_BUILD_UNIT_TESTS OFF CACHE BOOL "" FORCE)
set(BOX2D_BUILD_TESTBED OFF CACHE BOOL "" FORCE)
set(BOX2D_BUILD_DOCS OFF CACHE BOOL "" FORCE)
FetchContent_Declare(box2d
  GIT_REPOSITORY https://github.com/erincatto/box2d.git
  GIT_TAG        v2.4.1
  GIT_SHALLOW    TRUE)

# Dear ImGui (sources only) + ImGui-SFML (branch for SFML 2.6).
FetchContent_Declare(imgui
  GIT_REPOSITORY https://github.com/ocornut/imgui.git
  GIT_TAG        v1.89.9
  GIT_SHALLOW    TRUE)
FetchContent_Declare(imgui-sfml
  GIT_REPOSITORY https://github.com/SFML/imgui-sfml.git
  GIT_TAG        2.6.x
  GIT_SHALLOW    TRUE)

# nlohmann/json 3.11.3 - official release tarball (smaller than the git repo).
FetchContent_Declare(json
  URL https://github.com/nlohmann/json/releases/download/v3.11.3/json.tar.xz
  DOWNLOAD_EXTRACT_TIMESTAMP TRUE)

# spdlog 1.14.1
FetchContent_Declare(spdlog
  GIT_REPOSITORY https://github.com/gabime/spdlog.git
  GIT_TAG        v1.14.1
  GIT_SHALLOW    TRUE)

FetchContent_MakeAvailable(SFML box2d imgui json spdlog)

# ImGui-SFML needs to know where ImGui lives and must reuse our SFML targets.
set(IMGUI_DIR ${imgui_SOURCE_DIR})
set(IMGUI_SFML_FIND_SFML OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(imgui-sfml)

# ---------------------------------------------------------------------------
# Application
# ---------------------------------------------------------------------------
add_executable(physics_sandbox
  src/main.cpp
  src/Application.cpp
  src/DebrisEffect.cpp
  src/Logging.cpp
  src/PhysicsWorld.cpp
  src/Renderer.cpp
  src/SceneSerializer.cpp
  src/ShapeFactory.cpp
  src/Shapes.cpp
  src/UiPanel.cpp
  src/VelocityTracker.cpp)

target_include_directories(physics_sandbox PRIVATE include)

target_link_libraries(physics_sandbox PRIVATE
  box2d
  sfml-graphics
  sfml-window
  sfml-system
  ImGui-SFML::ImGui-SFML
  nlohmann_json::nlohmann_json
  spdlog::spdlog)

if(MSVC)
  target_compile_options(physics_sandbox PRIVATE /W4)
else()
  target_compile_options(physics_sandbox PRIVATE -Wall -Wextra -Wpedantic)
endif()
