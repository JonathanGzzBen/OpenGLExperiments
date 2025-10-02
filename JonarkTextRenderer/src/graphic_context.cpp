#include "jtr/graphic_context.h"

#include <iostream>
#include <print>

static GraphicContext g_graphic_context = {.valid = false};

// This function should only be called once per program
auto graphic_context_create(const GraphicContextConfig &config)
    -> GraphicContext {
  if (g_graphic_context.valid) {
    std::println(std::cerr, "Graphic context already exists");
    return g_graphic_context;
  }
  if (MAX_WINDOW_HINTS < config.glfw_window_hints_count) {
    std::println(std::cerr,
                 "Number of glfw window hints exceeds MAX_WINDOW_HINTS({})",
                 MAX_WINDOW_HINTS);
    return GraphicContext{.valid = false};
  }
  // Initialize glfw
  if (glfwInit() != GLFW_TRUE) {
    std::println(std::cerr, "Could not initialize glfw");
    return GraphicContext{.valid = false};
  }
  // Set glfw error callback
  glfwSetErrorCallback(config.glfw_error_callback);

  // Set glfw window hints
  for (size_t i = 0; i < config.glfw_window_hints_count; ++i) {
    glfwWindowHint(config.glfw_window_hints[i].key,
                   config.glfw_window_hints[i].value);
  }

  GLFWwindow *window =
      glfwCreateWindow(config.window_width, config.window_height,
                       config.window_title, nullptr, nullptr);
  if (window == nullptr) {
    std::println(std::cerr, "Could not create GLFW window");
    glfwTerminate();
    return GraphicContext{.valid = false};
  }

  glfwMakeContextCurrent(window);

  if (glewInit() != GLEW_OK) {
    std::println(std::cerr, "Could not initialize GLEW");
    glfwTerminate();
    return GraphicContext{.valid = false};
  }

  // Configure OpenGL debugging if Debug context was created
  if (config.gl_enable_debug) {
    int context_flags;
    glGetIntegerv(GL_CONTEXT_FLAGS, &context_flags);
    if ((context_flags & GL_CONTEXT_FLAG_DEBUG_BIT) != 0) {
      glEnable(GL_DEBUG_OUTPUT);
      glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
      glDebugMessageCallback(config.gl_debug_callback, nullptr);
      glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0,
                            nullptr, GL_TRUE);
    }
  }

  const auto context = GraphicContext{.valid = true,
                                      .window = window,
                                      .window_width = config.window_width,
                                      .window_height = config.window_height};
  g_graphic_context = context;
  return g_graphic_context;
}

auto graphic_context_destroy(GraphicContext &graphic_context) -> void {
  if (!graphic_context.valid) {
    return;
  }
  glfwDestroyWindow(graphic_context.window);
  glfwTerminate();
  graphic_context.valid = false;
}
