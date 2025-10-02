#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <functional>
#include <iostream>
#include <print>

#include "jtr/graphic_context.h"
#include "jtr/mesh.h"

auto glfw_error_callback(int error, const char *description) -> void {
  std::println(std::cerr, "GLFW error {}: {}", error, description);
}

void APIENTRY gl_debug_callback(GLenum source, GLenum type, GLuint debug_id,
                                GLenum severity, GLsizei length,
                                const GLchar *message, const void *userParam) {
  if (severity != GL_DEBUG_SEVERITY_NOTIFICATION) {
    std::println(std::cerr, "glDebugCallback: {}: {}", type, message);
  }
}

auto main() -> int {
  constexpr GraphicContextConfig config{
      .glfw_error_callback = glfw_error_callback,
      .glfw_window_hints =
          {
              {GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE},
              {GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE},
              {GLFW_CONTEXT_VERSION_MAJOR, 4},
              {GLFW_CONTEXT_VERSION_MINOR, 5},
              {GLFW_OPENGL_DEBUG_CONTEXT, GLFW_TRUE},
          },
      .glfw_window_hints_count = 5,
      .gl_debug_callback = gl_debug_callback,
      .gl_enable_debug = true,
      .window_title = "Jonark",
      .window_width = 600,
      .window_height = 600,
  };
  auto graphic_context = graphic_context_create(config);
  if (!graphic_context.valid) {
    std::println(std::cerr, "Could not create Graphic context");
    return 1;
  }

  auto mesh_manager = mesh_manager_create(2);
  if (!mesh_manager.valid) {
    std::println(std::cerr, "Could not create Mesh manager");
    return 1;
  }
  mesh_manager_destroy_all(mesh_manager);

  glEnable(GL_DEPTH_TEST);
  /* Enable alpha blend for font */
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glBlendEquation(GL_FUNC_ADD);
  glClearColor(0.0F, 0.0F, 0.0F, 1.0F);
  while (glfwWindowShouldClose(graphic_context.window) == 0) {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Render
    glfwSwapBuffers(graphic_context.window);
    glfwPollEvents();
  }

  graphic_context_destroy(graphic_context);
  return 0;
}