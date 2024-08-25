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

FetchContent_Declare(
  assimp
  GIT_REPOSITORY https://github.com/assimp/assimp
  GIT_TAG 10df90ec144179f97803a382e4f07c0570665864 # 5.4.1
)

FetchContent_MakeAvailable(glfw glew glm assimp)

add_executable(MeshesModelLoading main.cpp ${CMAKE_SOURCE_DIR}/libs/gl_strings/gl_strings.cpp)
target_compile_definitions(MeshesModelLoading PRIVATE EXPERIMENT_NAME="MeshesModelLoading")
target_link_libraries(MeshesModelLoading glfw libglew_static glm::glm assimp)
set_target_properties(MeshesModelLoading PROPERTIES CXX_STANDARD 20)
target_include_directories(MeshesModelLoading PRIVATE ${CMAKE_SOURCE_DIR}/libs)

if(WIN32 AND CMAKE_BUILD_TYPE STREQUAL "Release")
  set_target_properties(MeshesModelLoading
    PROPERTIES WIN32_EXECUTABLE YES
    LINK_FLAGS "/ENTRY:mainCRTStartup")
endif()
