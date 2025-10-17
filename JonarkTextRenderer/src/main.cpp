#define STB_IMAGE_WRITE_IMPLEMENTATION
#define STB_TRUETYPE_IMPLEMENTATION
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <fstream>
#include <functional>
#include <iostream>
#include <print>

#include "jtr/font.h"
#include "jtr/graphic_context.h"
#include "jtr/mesh.h"
#include "jtr/program.h"
#include "jtr/text.h"
#include "jtr/texture.h"
#include "jtr/vertex_array_object.h"

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

auto read_file(const char *filename) -> std::unique_ptr<char[]> {
  std::ifstream stream(filename, std::ios::binary);
  if (!stream.is_open()) {
    std::println(std::cerr, "Could not open file {}", filename);
    return nullptr;
  }
  stream.seekg(0, std::ios::end);
  const auto size = static_cast<size_t>(stream.tellg());
  stream.seekg(0, std::ios::beg);
  auto buffer = std::make_unique<char[]>(size + 1);
  stream.read(buffer.get(), static_cast<long long>(size));
  buffer[size] = '\0';
  return buffer;
}

// For single parameter
template <typename T, typename Constructor, typename Deleter>
auto get_smart_manager(Constructor constructor, int &&max_num,
                       Deleter deleter) {
  return std::unique_ptr<T, Deleter>(new T(constructor(max_num)), deleter);
}

// Variadic
// template <typename T, typename Constructor, typename Deleter, typename...
// Args> auto get_smart_manager(Constructor constructor, Deleter deleter,
//                        Args &&...args) {
//   return std::unique_ptr<T, Deleter>(
//       new T(constructor(std::forward<Args>(args)...)), deleter);
// }

auto main() -> int {
  static constexpr GraphicContextConfig config{
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
  const auto graphic_context =
      std::unique_ptr<GraphicContext, decltype(&graphic_context_destroy)>(
          new GraphicContext(graphic_context_create(config)),
          graphic_context_destroy);
  if (!graphic_context->valid) {
    std::println(std::cerr, "Could not create Graphic context");
    return 1;
  }

  auto program_manager = get_smart_manager<ProgramManager>(
      program_manager_create, 1, program_manager_destroy_all);
  if (!program_manager->valid) {
    std::println(std::cerr, "Could not create Mesh manager");
    return 1;
  }

  static constexpr int font_atlas_width = 1024;
  static constexpr int font_atlas_height = 1024;
  auto font_manager = get_smart_manager<FontManager>(font_manager_create, 1,
                                                     font_manager_destroy_all);
  const auto font_handle = [&font_manager]() {
    const auto font_data = read_file("fonts/arial.ttf");
    return font_create(font_manager.get(),
                       reinterpret_cast<const unsigned char *>(font_data.get()),
                       32, 95, 64.0F, font_atlas_width, font_atlas_height);
  }();
  if (font_handle < 0) {
    std::println(stderr, "Could not load font");
    return 1;
  }
  const auto *const font = font_get(*font_manager, font_handle);

  const auto texture_manager = get_smart_manager<TextureManager>(
      texture_manager_create, 2, texture_manager_destroy_all);
  const auto font_atlas_texture_handle = texture_create(
      *texture_manager, font->bitmap, font_atlas_width, font_atlas_height);
  if (font_atlas_texture_handle < 0) {
    std::println(std::cerr, "Could not create Texture");
    return 1;
  }

  const auto program_handle = [&program_manager]() {
    const auto vertex_shader_source =
        std::unique_ptr<const char[]>(read_file("shaders/vertex.glsl"));
    const auto fragment_shader_source =
        std::unique_ptr<const char[]>(read_file("shaders/fragment.glsl"));

    return program_create(*program_manager, vertex_shader_source.get(),
                          fragment_shader_source.get());
  }();
  if (program_handle < 0) {
    std::println(std::cerr, "Could not create Program");
    return 1;
  }

  const auto vao_manager = get_smart_manager<VertexArrayObjectManager>(
      vertex_array_object_manager_create, 1,
      vertex_array_object_manager_destroy_all);

  const std::vector attributes = {
      VertexArrayAttributeEntry{.index = 0,
                                .size = 3,
                                .type = GL_FLOAT,
                                .normalized = GL_FALSE,
                                .relative_offset = 0,
                                .binding_index = 0},
      VertexArrayAttributeEntry{.index = 1,
                                .size = 2,
                                .type = GL_FLOAT,
                                .normalized = GL_FALSE,
                                .relative_offset = offsetof(Vertex, uv),
                                .binding_index = 0}};
  const auto vao_handle = vertex_array_object_create(
      vao_manager.get(), attributes.data(), attributes.size());
  if (vao_handle < 0) {
    std::println(std::cerr, "Could not create VAO");
    return 1;
  }

  const auto mesh_manager = get_smart_manager<MeshManager>(
      mesh_manager_create, 3, mesh_manager_destroy_all);
  if (!mesh_manager->valid) {
    std::println(std::cerr, "Could not create Mesh manager");
    return 1;
  }

  // Top-right, top-left, bottom-left, bottom-right
  /*
  static constexpr Vertex vertices[] = {
      {.position = glm::vec3(0.5F, 0.5F, 0.0F), .uv = glm::vec2(1.0F, 0.0F)},
      {.position = glm::vec3(-0.5F, 0.5F, 0.0F), .uv = glm::vec2(0.0F, 0.0F)},
      {.position = glm::vec3(-0.5F, -0.5F, 0.0F), .uv = glm::vec2(0.0F, 1.0F)},
      {.position = glm::vec3(0.5F, -0.5F, 0.0F), .uv = glm::vec2(1.0F, 1.0F)}};
  static constexpr unsigned int indices[] = {0, 1, 2, 0, 2, 3};
  constexpr MeshData font_atlas_mesh_data = {
      .valid = true,
      .vertices = vertices,
      .num_vertices = sizeof(vertices) / sizeof(Vertex),
      .indices = indices,
      .num_indices = sizeof(indices) / sizeof(unsigned int),
  };
  const auto font_atlas_mesh_handle = mesh_create(*mesh_manager, font_atlas_mesh_data);
  */

  static constexpr float pixel_scale = 2.0F / 600.0F;
  const auto text_mesh_handle =
      text_create_mesh(*font, mesh_manager.get(), glm::vec2(0.0F, 0.0F), "Hola",
                       1.0F, pixel_scale);
  if (text_mesh_handle < 0) {
    std::println(stderr, "Could not create text_mesh");
    return 1;
  }
  const auto second_text_mesh_handle =
      text_create_mesh(*font, mesh_manager.get(), glm::vec2(0.0F, 0.5F),
                       "XDDDD", 1.0F, pixel_scale);
  if (second_text_mesh_handle < 0) {
    std::println(stderr, "Could not create second_text_mesh");
    return 1;
  }

  glEnable(GL_DEPTH_TEST);
  /* Enable alpha blend for font */
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glBlendEquation(GL_FUNC_ADD);
  glClearColor(0.0F, 0.0F, 0.0F, 1.0F);
  while (glfwWindowShouldClose(graphic_context->window) == 0) {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    program_use(*program_manager, program_handle);

    // Draw font atlas
    // const auto *const mesh = mesh_get(*mesh_manager, mesh_handle);
    // glBindVertexArray(vao->vao_id);
    // glBindVertexBuffer(0, mesh->vbo, 0, sizeof(Vertex));
    // glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->ebo);

    static constexpr int font_atlas_texture_unit = 0;
    static constexpr auto font_atlas_texture_uniform_name = "font_atlas";
    texture_bind(*texture_manager, font_atlas_texture_handle, font_atlas_texture_unit);
    program_set_uniform(*program_manager, program_handle,
                        font_atlas_texture_uniform_name,
                        font_atlas_texture_unit);

    vertex_array_object_bind(*vao_manager, vao_handle);
    mesh_draw(*mesh_manager, text_mesh_handle);
    mesh_draw(*mesh_manager, second_text_mesh_handle);

    // Render
    glfwSwapBuffers(graphic_context->window);
    glfwPollEvents();
  }

  return 0;
}
