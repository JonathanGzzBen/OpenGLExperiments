#include <cstdlib>
#include <exception>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

#define GLFW_INCLUDE_NONE
#include <GL/glew.h>
#include <GLFW/glfw3.h>

auto error_callback(int error, const char* description) {
  std::cerr << "Error " << error << ": " << description << "\n";
}

static void key_callback(GLFWwindow* window, int key, int scancode, int action,
                         int mods) {
  if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
    glfwSetWindowShouldClose(window, GLFW_TRUE);
  }
}

auto GetShaderFromFile(const std::string& filename, GLenum type)
    -> unsigned int {
  const auto read_file = [](const std::string& filename) -> std::string {
    auto input = std::ifstream(filename, std::ios::binary);
    if (!input.is_open()) {
      throw std::runtime_error("Could not read file " + filename + "\n");
    }
    std::stringstream strBuf;
    strBuf << input.rdbuf();
    return strBuf.str();
  };
  const auto shader = glCreateShader(type);
  const auto source_code = read_file(filename);
  const auto* const source_pointer = source_code.c_str();
  glShaderSource(shader, 1, &(source_pointer), nullptr);

  glCompileShader(shader);
  int compilation_succesful = 0;
  glGetShaderiv(shader, GL_COMPILE_STATUS, &compilation_succesful);
  if (compilation_succesful == 0) {
    GLsizei log_length = 0;
    const unsigned int buf_len = 1024;
    GLchar message[buf_len];
    glGetShaderInfoLog(shader, buf_len, &log_length, message);
    std::cerr << "Error compiling " << filename << ": " << message;
  }
  return shader;
}

auto main() -> int {
  if (glfwInit() != GLFW_TRUE) {
    std::cerr << "Initialization failed\n";
    std::exit(EXIT_FAILURE);
  }

  glfwWindowHint(GLFW_VERSION_MAJOR, 4);
  glfwWindowHint(GLFW_VERSION_MINOR, 5);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  GLFWwindow* window =
      glfwCreateWindow(640, 480, EXPERIMENT_NAME, nullptr, nullptr);

  if (window == nullptr) {
    std::cerr << "Window creation failed\n";
    std::exit(EXIT_FAILURE);
  }

  glfwSetKeyCallback(window, key_callback);

  glfwMakeContextCurrent(window);
  glfwSwapInterval(1);  // Enable vsync

  const auto glew_err = glewInit();
  if (GLEW_OK != glew_err) {
    fprintf(stderr, "Glew Error: %s\n", glewGetErrorString(glew_err));
  }

  const auto vertex_shader =
      GetShaderFromFile("shaders/vertex.glsl", GL_VERTEX_SHADER);
  const auto fragment_shader =
      GetShaderFromFile("shaders/fragment.glsl", GL_FRAGMENT_SHADER);

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
    std::cerr << "Error linking program " << program << ": " << message;
  }
  glUseProgram(program);

  unsigned int vaos[1];
  glCreateVertexArrays(1, vaos);
  unsigned int vbos[1];
  glCreateBuffers(1, vbos);

  const float data[3][2] = {{-0.5F, -0.5F}, {0.0F, 0.5F}, {0.5F, -0.5F}};

  glDeleteVertexArrays(1, vaos);
  glDeleteVertexArrays(1, vbos);
  glNamedBufferStorage(vbos[0], sizeof(data), data, 0);

  glBindVertexArray(vaos[0]);
  glBindBuffer(GL_ARRAY_BUFFER, vbos[0]);

  glVertexAttribPointer(0, 2, GL_FLOAT, 0, 0, nullptr);
  glEnableVertexAttribArray(0);

  while (glfwWindowShouldClose(window) == GLFW_FALSE) {
    static const float black[] = {0.0F, 0.0F, 0.0F, 0.0F};

    glClearBufferfv(GL_COLOR, 0, black);

    glDrawArrays(GL_TRIANGLES, 0, 3);

    // Keep running
    glfwSwapBuffers(window);
    glfwPollEvents();
  }

  glfwDestroyWindow(window);
  glfwTerminate();
  return 0;
}
