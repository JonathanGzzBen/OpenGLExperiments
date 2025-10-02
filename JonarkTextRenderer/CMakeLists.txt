cmake_minimum_required(VERSION 3.11)

project(JonarkTextRenderer)

find_package(GLEW required)
find_package(glfw3 CONFIG REQUIRED)
find_package(glm CONFIG REQUIRED)
find_package(assimp CONFIG REQUIRED)
find_package(Stb REQUIRED)

add_executable(JonarkTextRenderer src/main.cpp src/graphic_context.cpp)
set_target_properties(JonarkTextRenderer PROPERTIES CXX_STANDARD 23 CXX_STANDARD_REQUIRED ON)
target_include_directories(JonarkTextRenderer PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/lib/include" ${Stb_INCLUDE_DIR})
target_link_libraries(JonarkTextRenderer PRIVATE GLEW::GLEW glfw glm::glm assimp::assimp)