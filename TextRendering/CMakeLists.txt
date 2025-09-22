cmake_minimum_required(VERSION 3.11)
project(TextRendering)

include(FetchContent)

find_package(OpenGL REQUIRED)

# ---- glfw ----
option(CAMERA_CONTROL_FETCH_GLFW "Use FetchContent for glfw (overrides vcpkg)" OFF)
if (CAMERA_CONTROL_FETCH_GLFW)
    message(STATUS "Using FetchContent for glfw")
    include(FetchContent)
    FetchContent_Declare(
            glfw
            GIT_REPOSITORY https://github.com/glfw/glfw
            GIT_TAG 7b6aead9fb88b3623e3b3725ebb42670cbe4c579 # 3.4.0
            FIND_PACKAGE_ARGS
    )
    set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
    set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
    set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)

    FetchContent_MakeAvailable(glfw)
else ()
    message(STATUS "Using find_package(glfw3)")
    find_package(glfw3 REQUIRED)
endif ()

# ---- glew ----
option(CAMERA_CONTROL_FETCH_GLEW "Use FetchContent for glew (overrides vcpkg)" OFF)
if (CAMERA_CONTROL_FETCH_GLEW)
    message(STATUS "Using FetchContent for glew")
    include(FetchContent)
    FetchContent_Declare(
            glew
            GIT_REPOSITORY https://github.com/Perlmint/glew-cmake
            GIT_TAG f456deace7b408655109aaeff71421ef2d3858c6 # 2.2.0
            FIND_PACKAGE_ARGS
    )
    FetchContent_MakeAvailable(glew)
else ()
    message(STATUS "Using find_package(glew)")
    find_package(glew REQUIRED)
endif ()

# ---- glm ----
option(CAMERA_CONTROL_FETCH_GLM "Use FetchContent for glm (overrides vcpkg)" OFF)
if (CAMERA_CONTROL_FETCH_GLM)
    message(STATUS "Using FetchContent for glm")
    include(FetchContent)
    FetchContent_Declare(
            glm
            GIT_REPOSITORY https://github.com/g-truc/glm
            GIT_TAG bf71a834948186f4097caa076cd2663c69a10e1e # refs/tags/1.0.1
    )
    FetchContent_MakeAvailable(glm)
else ()
    message(STATUS "Using find_package(glm)")
    find_package(glm REQUIRED)
endif ()

# ---- CLI11 ----
option(CAMERA_CONTROL_FETCH_CLI11 "Use FetchContent for CLI11 (overrides vcpkg)" OFF)
if (CAMERA_CONTROL_FETCH_CLI11)
    message(STATUS "Using FetchContent for CLI11")
    include(FetchContent)
    FetchContent_Declare(
            CLI11
            GIT_REPOSITORY https://github.com/CLIUtils/CLI11.git
            GIT_TAG v2.3.2
    )
    FetchContent_MakeAvailable(CLI11)
else ()
    message(STATUS "Using find_package(CLI11)")
    find_package(CLI11 REQUIRED)
endif ()

add_library(text_rendering_lib STATIC
        lib/model_loading/src/program.cpp
        lib/model_loading/src/mesh.cpp
        lib/model_loading/src/model.cpp)
target_include_directories(text_rendering_lib PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/lib/model_loading/include)
target_link_libraries(text_rendering_lib PUBLIC GLEW::GLEW)
set_target_properties(text_rendering_lib PROPERTIES CXX_STANDARD 23)
set_target_properties(text_rendering_lib PROPERTIES CXX_STANDARD_REQUIRED ON)

find_package(Stb REQUIRED)

find_package(assimp CONFIG REQUIRED)

add_executable(text_rendering src/main.cpp
        lib/model_loading/include/program.h)

target_link_libraries(text_rendering text_rendering_lib glfw GLEW::GLEW glm::glm CLI11::CLI11 assimp::assimp)
target_include_directories(text_rendering PRIVATE ${CMAKE_SOURCE_DIR}/include ${Stb_INCLUDE_DIR})
set_target_properties(text_rendering PROPERTIES CXX_STANDARD 23)
set_target_properties(text_rendering PROPERTIES CXX_STANDARD_REQUIRED ON)
