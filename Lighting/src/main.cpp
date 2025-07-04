#include <iostream>

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include "program.h"

void APIENTRY glDebugCallback(GLenum source, GLenum type, GLuint id,
                              GLenum severity, GLsizei length,
                              const GLchar* message, const void* userParam) {
  std::cerr << "OpenGL debug: " << message << std::endl;
}

void glfwErrorCallback(const int error, const char* description) {
  std::cerr << "GLFW error: " << description << std::endl;
}

auto main() -> int {
  if (glfwInit() != GLFW_TRUE) {
    std::cerr << "Failed to initialize!" << std::endl;
    return 1;
  }

  glfwSetErrorCallback(glfwErrorCallback);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
  glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);

  GLFWwindow* window = glfwCreateWindow(800, 600, "Lighting", nullptr, nullptr);
  if (window == nullptr) {
    std::cerr << "Failed to create GLFW window!" << std::endl;
    glfwTerminate();
    return 1;
  }

  glfwMakeContextCurrent(window);

  if (glewInit() != GLEW_OK) {
    std::cerr << "Failed to initialize GLEW!" << std::endl;
    return 1;
  }

  const auto program = lighting::Program::Create(
      "shaders/vertex.glsl", "shaders/fragment.glsl");
  if (!program) {
    std::cerr << "Failed to initialize program: " << program.error().message <<
        std::endl;
    glfwTerminate();
    return 1;
  }

  using Vertex = struct Vertex {
    float x, y, z;
  };

  std::vector<Vertex> vertices = {
      {.x = -0.5F, .y = -0.5F, .z = 0.0F},
      {.x = -0.5F, .y = 0.5F, .z = 0.0F},
      {.x = 0.5F, .y = 0.5F, .z = 0.0F},
  };

  unsigned int vao;
  glCreateVertexArrays(1, &vao);
  glBindVertexArray(vao);
  unsigned int vbo;
  glGenBuffers(1, &vbo);
  glBindBuffer(GL_ARRAY_BUFFER, vbo);
  glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * vertices.size(),
               vertices.data(), GL_STATIC_DRAW);
  glVertexAttribPointer(0, sizeof(Vertex) / sizeof(float), GL_FLOAT, GL_FALSE,
                        sizeof(Vertex), nullptr);
  glEnableVertexAttribArray(0);
  glBindVertexArray(0);

  glClearColor(0.0F, 0.0F, 0.0F, 1.0F);
  while (glfwWindowShouldClose(window) != GLFW_TRUE) {
    glClear(GL_COLOR_BUFFER_BIT);
    program->Use();

    glBindVertexArray(vao);
    glDrawArrays(GL_TRIANGLES, 0, 3);

    glfwSwapBuffers(window);
    glfwPollEvents();
  }
  glfwTerminate();
  return 0;
}