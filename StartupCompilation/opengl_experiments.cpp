#include <cstdlib>
#include <iostream>

#define GLFW_INCLUDE_NONE
#include <GL/glew.h>
#include <GLFW/glfw3.h>

auto error_callback(int error, const char *description) {
  std::cerr << "Error " << error << ": " << description << "\n";
}

static void key_callback(GLFWwindow *window, int key, int scancode, int action,
                         int mods) {
  if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
    glfwSetWindowShouldClose(window, GLFW_TRUE);
  }
}

template <typename... Args>
static void printError(Args... args) noexcept {
  try {
    (std::cerr << ... << args);
  } catch (const std::exception &e) {
    std::fprintf(stderr, "printError failed: %s", e.what());
  }
}

auto main() -> int {
  if (glfwInit() != GLFW_TRUE) {
    printError("Initialization failed\n");
    std::exit(EXIT_FAILURE);
  }

  glfwSetErrorCallback(error_callback);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  GLFWwindow *window =
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
    fprintf(stderr, "Glew Error: %s\n", glewGetErrorString(glew_err));
  }

  while (glfwWindowShouldClose(window) == GLFW_FALSE) {
    // Keep running
    glfwSwapBuffers(window);
    glfwPollEvents();
  }

  glfwDestroyWindow(window);
  glfwTerminate();
  return 0;
}
