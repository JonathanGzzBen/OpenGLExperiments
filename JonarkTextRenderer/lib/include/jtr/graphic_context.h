#ifndef GRAPHIC_CONTEXT_H
#define GRAPHIC_CONTEXT_H

#include <GL/glew.h>
#include <GLFW/glfw3.h>

using WindowHint = struct WindowHint {
  int key;
  int value;
};
static constexpr int MAX_WINDOW_HINTS = 5;

using GraphicContextConfig = struct GraphicContextConfig {
  void (*glfw_error_callback)(int error, const char *description);
  WindowHint glfw_window_hints[MAX_WINDOW_HINTS];
  unsigned int glfw_window_hints_count;
  void (*gl_debug_callback)(GLenum source, GLenum type, GLuint id,
                            GLenum severity, GLsizei length,
                            const GLchar *message, const void *userParam);
  bool gl_enable_debug;
  const char *window_title;
  int window_width;
  int window_height;
};

using GraphicContext = struct GraphicContext {
  bool valid;
  GLFWwindow *window;
  int window_width;
  int window_height;
};

// This function should only be called once per program
auto graphic_context_create(const GraphicContextConfig &config)
    -> GraphicContext;

auto graphic_context_destroy(GraphicContext *graphic_context) -> void;

#endif  // GRAPHIC_CONTEXT_H
