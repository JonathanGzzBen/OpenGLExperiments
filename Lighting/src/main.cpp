#include <iostream>

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/ext/matrix_common.hpp>
#include <vector>
#include "program.h"

void APIENTRY glDebugCallback(GLenum source, GLenum type, GLuint id,
                              GLenum severity, GLsizei length,
                              const GLchar* message, const void* userParam) {
  if (severity != GL_DEBUG_SEVERITY_NOTIFICATION) {
    std::cerr << "OpenGL debug: " << message << "\n";
  }
}

void glfwErrorCallback(const int error, const char* description) {
  std::cerr << "GLFW error: " << description << "\n";
}

auto main() -> int {
  if (glfwInit() != GLFW_TRUE) {
    std::cerr << "Failed to initialize!\n";
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
    std::cerr << "Failed to create GLFW window!\n";
    glfwTerminate();
    return 1;
  }

  glfwMakeContextCurrent(window);

  if (glewInit() != GLEW_OK) {
    std::cerr << "Failed to initialize GLEW!\n";
    return 1;
  }

  int flags;
  glGetIntegerv(GL_CONTEXT_FLAGS, &flags);
  if ((flags & GL_CONTEXT_FLAG_DEBUG_BIT) != 0) {
    glEnable(GL_DEBUG_OUTPUT);
    glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
    glDebugMessageCallback(glDebugCallback, nullptr);
    glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr,
                          GL_TRUE);
  }

  const auto program = lighting::Program::Create(
      "shaders/vertex.glsl", "shaders/fragment.glsl");
  if (!program) {
    std::cerr << "Failed to initialize program: " << program.error().message <<
        "\n";

    glfwTerminate();
    return 1;
  }

  using Vertex = struct Vertex {
    float x, y, z, u, v, nx, ny, nz;
  };

  std::vector<Vertex> vertices = {
      {.x = -0.5F, .y = -0.5F, .z = 0.0F, .u = 0.0F, .v = 0.0F, .nx = 0.0F,
       .ny = 0.0F, .nz = 0.0F},
      {.x = -0.5F, .y = 0.5F, .z = 0.0F, .u = 0.0F, .v = 0.0F, .nx = 0.0F,
       .ny = 0.0F, .nz = 0.0F},
      {.x = 0.5F, .y = 0.5F, .z = 0.0F, .u = 0.0F, .v = 0.0F, .nx = 0.0F,
       .ny = 0.0F, .nz = 0.0F},
      {.x = 0.5F, .y = -0.5F, .z = 0.0F, .u = 0.0F, .v = 0.0F, .nx = 0.0F,
       .ny = 0.0F, .nz = 0.0F},
  };

  unsigned int vao;
  glCreateVertexArrays(1, &vao);
  glEnableVertexArrayAttrib(vao, 0);
  glEnableVertexArrayAttrib(vao, 1);
  glEnableVertexArrayAttrib(vao, 2);

  glVertexArrayAttribBinding(vao, 0, 0);
  glVertexArrayAttribBinding(vao, 1, 0);
  glVertexArrayAttribBinding(vao, 2, 0);
  glVertexArrayAttribFormat(vao, 0, 3, GL_FLOAT, GL_FALSE, offsetof(Vertex, x));
  glVertexArrayAttribFormat(vao, 1, 2, GL_FLOAT, GL_FALSE, offsetof(Vertex, u));
  glVertexArrayAttribFormat(vao, 2, 3, GL_FLOAT, GL_FALSE,
                            offsetof(Vertex, nx));

  unsigned int vbo;
  glCreateBuffers(1, &vbo);
  glNamedBufferStorage(
      vbo, static_cast<GLsizei>(vertices.size() * sizeof(Vertex)),
      vertices.data(),
      0);

  unsigned int ebo;
  glCreateBuffers(1, &ebo);
  std::vector<unsigned int> indices = {0, 1, 2, 2, 3, 0};
  glNamedBufferStorage(
      ebo, static_cast<GLsizei>(indices.size() * sizeof(unsigned int)),
      indices.data(), 0);

  glClearColor(0.0F, 0.0F, 0.0F, 1.0F);
  while (glfwWindowShouldClose(window) != GLFW_TRUE) {
    glClear(GL_COLOR_BUFFER_BIT);
    program->Use();

    glBindVertexArray(vao);
    glVertexArrayVertexBuffer(vao, 0, vbo, 0, sizeof(Vertex));
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);

    glfwSwapBuffers(window);
    glfwPollEvents();
  }
  glfwTerminate();
  return 0;
}