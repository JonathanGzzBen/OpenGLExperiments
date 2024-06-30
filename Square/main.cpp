#include <cstdlib>
#include <exception>
#include <fstream>
#include <iostream>
#include <optional>
#include <sstream>
#include <string>

#define GLFW_INCLUDE_NONE
#include <GL/glew.h>
#include <GLFW/glfw3.h>

template <typename... Args>
static void printError(Args... args) noexcept {
  try {
    (std::cerr << ... << args);
  } catch (const std::exception& e) {
    std::fprintf(stderr, "printError failed: %s", e.what());
  }
}

auto glfw_error_callback(int error, const char* description) {
  printError("Error ", error, ": ", description, "\n");
}

static void key_callback(GLFWwindow* window, int key, int scancode, int action,
                         int mods) {
  if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
    glfwSetWindowShouldClose(window, GLFW_TRUE);
  }
}

auto ReadFile(const std::string& filename) -> std::string {
  const std::ifstream input(filename, std::ios::binary);
  std::stringstream strBuf;
  strBuf << input.rdbuf();
  if (input.fail() && !input.eof()) {
    printError("Error reading file \"", filename, "\"\n");
    return "";
  }
  return strBuf.str();
};

auto GetShaderFromFile(const std::string& filename, GLenum type) noexcept
    -> unsigned int {
  std::string source_code;
  try {
    source_code = ReadFile(filename);
  } catch (const std::exception& ex) {
    printError("Error reading from file \"", filename.c_str(),
               "\": ", ex.what(), "\n");
    return 0;
  }

  if (source_code.empty()) {
    return 0;
  }

  const auto shader = glCreateShader(type);
  if (shader == 0) {
    return 0;
  }

  const auto* const source_pointer = source_code.data();
  glShaderSource(shader, 1, &(source_pointer), nullptr);

  glCompileShader(shader);
  int compilation_succesful = 0;
  glGetShaderiv(shader, GL_COMPILE_STATUS, &compilation_succesful);
  if (compilation_succesful == 0) {
    GLsizei log_length = 0;
    const unsigned int buf_len = 1024;
    GLchar message[buf_len];
    glGetShaderInfoLog(shader, buf_len, &log_length, message);
    printError("Error compiling ", filename, ": ", message, "\n");
  }
  return shader;
}

auto main() -> int {
  if (glfwInit() != GLFW_TRUE) {
    printError("Initialization failed\n");
    std::exit(EXIT_FAILURE);
  }

  glfwSetErrorCallback(glfw_error_callback);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  GLFWwindow* window =
      glfwCreateWindow(640, 480, EXPERIMENT_NAME, nullptr, nullptr);

  if (window == nullptr) {
    printError("Window creation failed\n");
    std::exit(EXIT_FAILURE);
  }

  glfwSetKeyCallback(window, key_callback);

  glfwMakeContextCurrent(window);
  glfwSwapInterval(1);  // Enable vsync

  const auto glew_err = glewInit();
  if (GLEW_OK != glew_err) {
    printError("Glew Error: ", glewGetErrorString(glew_err), "\n");
  }

  unsigned int vertex_shader =
      GetShaderFromFile("shaders/vertex.glsl", GL_VERTEX_SHADER);
  if (vertex_shader == 0) {
    printError("Error building VERTEX SHADER\n");
    exit(EXIT_FAILURE);
  }
  unsigned int fragment_shader =
      GetShaderFromFile("shaders/fragment.glsl", GL_FRAGMENT_SHADER);
  if (fragment_shader == 0) {
    printError("Error building FRAGMENT SHADER\n");
    exit(EXIT_FAILURE);
  }

  const auto program = glCreateProgram();
  glAttachShader(program, vertex_shader);
  glAttachShader(program, fragment_shader);
  glLinkProgram(program);
  int link_successful = 0;
  glGetProgramiv(program, GL_LINK_STATUS, &link_successful);
  if (link_successful == 0) {
    GLsizei log_length = 0;
    const unsigned int buf_len = 1024;
    GLchar message[buf_len];
    glGetProgramInfoLog(program, buf_len, &log_length, message);
    printError("Error linking program ", program, ": ", message, "\n");
  }
  glUseProgram(program);

  unsigned int vaos[1];
  glCreateVertexArrays(1, vaos);
  unsigned int vbos[1];
  glCreateBuffers(1, vbos);

  const float data[][2] = {{-0.5F, 0.5F},  {0.5F, 0.5F}, {-0.5F, -0.5F},
                           {-0.5F, -0.5F}, {0.5F, 0.5F}, {0.5F, -0.5F}};
  glNamedBufferStorage(vbos[0], sizeof(data), data, 0);

  glBindVertexArray(vaos[0]);
  glBindBuffer(GL_ARRAY_BUFFER, vbos[0]);

  glVertexAttribPointer(0, 2, GL_FLOAT, 0, 0, nullptr);
  glEnableVertexAttribArray(0);

  while (glfwWindowShouldClose(window) == GLFW_FALSE) {
    static const float black[] = {0.0F, 0.0F, 0.0F, 0.0F};

    glClearBufferfv(GL_COLOR, 0, black);

    glDrawArrays(GL_TRIANGLES, 0, 6);

    // Keep running
    glfwSwapBuffers(window);
    glfwPollEvents();
  }

  glDeleteBuffers(1, vbos);
  glDeleteVertexArrays(1, vaos);
  glfwDestroyWindow(window);
  glfwTerminate();
  return 0;
}
