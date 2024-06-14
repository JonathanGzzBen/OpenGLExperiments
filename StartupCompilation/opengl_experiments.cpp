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
  if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
    glfwSetWindowShouldClose(window, GLFW_TRUE);
}

auto main() -> int {
  if (!glfwInit()) {
    std::cerr << "Initialization failed\n";
    std::exit(EXIT_FAILURE);
  }

  GLFWwindow *window =
      glfwCreateWindow(640, 480, EXPERIMENT_NAME, nullptr, nullptr);

  if (!window) {
    std::cerr << "Window creation failed\n";
    std::exit(EXIT_FAILURE);
  }

  glfwSetKeyCallback(window, key_callback);

  glfwMakeContextCurrent(window);
  glfwSwapInterval(1);  // Enable vsync

  const auto glew_err = glewInit();
  glewExperimental = true;
  if (GLEW_OK != glew_err) {
    fprintf(stderr, "Glew Error: %s\n", glewGetErrorString(glew_err));
  }

  while (!glfwWindowShouldClose(window)) {
    // Keep running
    glfwSwapBuffers(window);
    glfwPollEvents();
  }

  glfwDestroyWindow(window);
  glfwTerminate();
  return 0;
}
