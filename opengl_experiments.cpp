#include <cstdlib>
#include <iostream>

#define GLFW_INCLUDE_NONE
#include <GL/glew.h>
#include <GLFW/glfw3.h>

auto error_callback(int error, const char* description) {
    std::cerr << "Error " << error << ": " << description << "\n";
}

static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GLFW_TRUE);
}

auto main() -> int {
    glewInit();
    if (!glfwInit()) {
        std::cerr << "Initialization failed\n";
        std::exit(EXIT_FAILURE);
    }

    GLFWwindow* window = glfwCreateWindow(640, 480, "OpenGL Experiments", nullptr, nullptr);

    if (!window) {
        std::cerr << "Window creation failed\n";
        std::exit(EXIT_FAILURE);

    }

    glfwSetKeyCallback(window, key_callback);

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1); // Enable vsync


    while (!glfwWindowShouldClose(window)) {
        // Keep running
        glfwSwapBuffers(window);
        glfwPollEvents();

    }

    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
