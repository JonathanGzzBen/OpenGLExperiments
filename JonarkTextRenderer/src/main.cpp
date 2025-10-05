#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <fstream>
#include <functional>
#include <iostream>
#include <print>

#include "jtr/graphic_context.h"
#include "jtr/mesh.h"
#include "jtr/program.h"

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

auto read_file(const char *filename) -> const char * {
  std::ifstream stream(filename, std::ios::binary);
  if (!stream.is_open()) {
    std::println(std::cerr, "Could not open file {}", filename);
    return nullptr;
  }
  stream.seekg(0, std::ios::end);
  const auto size = static_cast<size_t>(stream.tellg());
  stream.seekg(0, std::ios::beg);
  auto *buffer = new char[size + 1];
  stream.read(buffer, static_cast<long long>(size));
  buffer[size] = '\0';
  return buffer;
}

[[noreturn]]
auto terminate_application(int status_code, GraphicContext *graphic_context,
                           ProgramManager *program_manager,
                           MeshManager *mesh_manager) -> void {
  if (program_manager != nullptr && program_manager->valid) {
    program_manager_destroy_all(*program_manager);
  }
  if (mesh_manager != nullptr && mesh_manager->valid) {
    mesh_manager_destroy_all(*mesh_manager);
  }
  if (graphic_context != nullptr && graphic_context->valid) {
    graphic_context_destroy(*graphic_context);
  }
  exit(status_code);
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
    terminate_application(EXIT_FAILURE, &graphic_context, nullptr, nullptr);
  }

  auto program_manager = program_manager_create(1);
  if (!program_manager.valid) {
    std::println(std::cerr, "Could not create Mesh manager");
    terminate_application(EXIT_FAILURE, &graphic_context, &program_manager,
                          nullptr);
  }

  const auto *const vertex_shader_source = read_file("shaders/vertex.glsl");
  const auto *const fragment_shader_source = read_file("shaders/fragment.glsl");
  const auto program_handle = program_create(
      program_manager, vertex_shader_source, fragment_shader_source);
  delete[] vertex_shader_source;
  delete[] fragment_shader_source;
  if (program_handle < 0) {
    std::println(std::cerr, "Could not create Program");
    return 1;
  }

  unsigned int vao;
  glCreateVertexArrays(1, &vao);
  glBindVertexArray(vao);

  glVertexArrayAttribFormat(vao, 0, 3, GL_FLOAT, GL_FALSE, 0);
  glVertexArrayAttribFormat(vao, 1, 2, GL_FLOAT, GL_FALSE, 3);

  glVertexArrayAttribBinding(vao, 0, 0);
  glVertexArrayAttribBinding(vao, 1, 0);

  glEnableVertexArrayAttrib(vao, 0);
  glEnableVertexArrayAttrib(vao, 1);

  auto mesh_manager = mesh_manager_create(2);
  if (!mesh_manager.valid) {
    std::println(std::cerr, "Could not create Mesh manager");
    terminate_application(EXIT_FAILURE, &graphic_context, &program_manager,
                          &mesh_manager);
  }

  // Top-right, top-left, bottom-left, bottom-right
  Vertex vertices[] = {
      {.position = glm::vec3(0.5F, 0.5F, 0.0F), .uv = glm::vec2(0.0F, 0.0F)},
      {.position = glm::vec3(-0.5F, 0.5F, 0.0F), .uv = glm::vec2(0.0F, 0.0F)},
      {.position = glm::vec3(-0.5F, -0.5F, 0.0F), .uv = glm::vec2(0.0F, 0.0F)},
      {.position = glm::vec3(0.5F, -0.5F, 0.0F), .uv = glm::vec2(0.0F, 0.0F)}};
  unsigned int indices[] = {0, 1, 2, 0, 2, 3};
  MeshData mesh_data = {.valid = true,
                        .vertices = vertices,
                        .num_vertices = sizeof(vertices) / sizeof(Vertex),
                        .indices = indices,
                        .num_indices = sizeof(indices) / sizeof(unsigned int),
                        .texture_id = 0};
  const auto mesh_handle = mesh_create(mesh_manager, mesh_data);

  glEnable(GL_DEPTH_TEST);
  /* Enable alpha blend for font */
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glBlendEquation(GL_FUNC_ADD);
  glClearColor(0.0F, 0.0F, 0.0F, 1.0F);
  while (glfwWindowShouldClose(graphic_context.window) == 0) {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    auto *const program = program_get(program_manager, program_handle);
    if (program == nullptr || !program->valid) {
      std::println(std::cerr, "Could not get program");
      terminate_application(EXIT_FAILURE, &graphic_context, &program_manager,
                            &mesh_manager);
    }
    glUseProgram(program->program_id);

    auto *const mesh = mesh_get(mesh_manager, mesh_handle);
    glBindVertexArray(vao);
    glBindVertexBuffer(0, mesh->vbo, 0, sizeof(Vertex));
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->ebo);
    glDrawElements(GL_TRIANGLES, static_cast<int>(mesh->num_indices),
                   GL_UNSIGNED_INT, nullptr);

    // Render
    glfwSwapBuffers(graphic_context.window);
    glfwPollEvents();
  }

  terminate_application(EXIT_SUCCESS, &graphic_context, &program_manager,
                        &mesh_manager);
}