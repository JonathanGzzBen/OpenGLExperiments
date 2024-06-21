cmake_minimum_required(VERSION 3.11)

include(FetchContent)

find_package(OpenGL REQUIRED)

FetchContent_Declare(
  glfw
  GIT_REPOSITORY https://github.com/glfw/glfw
  GIT_TAG 7b6aead9fb88b3623e3b3725ebb42670cbe4c579 # 3.4.0
)

set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)

FetchContent_Declare(
  glew
  GIT_REPOSITORY https://github.com/Perlmint/glew-cmake
  GIT_TAG f456deace7b408655109aaeff71421ef2d3858c6 # 2.2.0
)

FetchContent_Declare(
  glm
  GIT_REPOSITORY https://github.com/g-truc/glm
  GIT_TAG bf71a834948186f4097caa076cd2663c69a10e1e # refs/tags/1.0.1
)

FetchContent_MakeAvailable(glfw glew glm)

add_executable(Textures main.cpp)
target_compile_definitions(Textures PRIVATE EXPERIMENT_NAME="Textures")
target_link_libraries(Textures glfw libglew_static glm::glm)
set_target_properties(Textures PROPERTIES CXX_STANDARD 20)
