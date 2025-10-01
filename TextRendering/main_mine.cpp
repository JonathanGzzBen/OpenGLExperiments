#define STB_TRUETYPE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <stb_image_write.h>
#include <stb_truetype.h>

#include <cstdio>
#include <cstring>
#include <fstream>
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/fwd.hpp>
#include <glm/gtc/type_ptr.inl>
#include <glm/mat4x4.hpp>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <print>
#include <vector>

static std::string vertex_shader_source = R"(
#version 330 core

layout (location = 0) in vec3 aPosition;
layout (location = 1) in vec4 aColor;
layout (location = 2) in vec2 aTexCoord;

out vec4 color;
out vec2 texCoord;

uniform mat4 uViewProjectionMat;

void main()
{
    gl_Position = uViewProjectionMat * vec4(aPosition, 1.0);

    color = aColor;
    texCoord = aTexCoord;
}

)";

static const std::string fragment_shader_source = R"(
#version 330 core

in vec4 color;
in vec2 texCoord;

uniform sampler2D uFontAtlasTexture;

out vec4 fragColor;

void main()
{
    fragColor = vec4(texture(uFontAtlasTexture, texCoord).r) * color;
}

)";

using Window = struct Window {
  bool valid;
  GLFWwindow* glfw_window;
  int width;
  int height;
};

auto initialize_window_with_context(const int width, const int height,
                                    const char* title,
                                    const bool request_debug_context)
    -> Window {
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
  glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);

  if (request_debug_context) {
    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);
  }
  GLFWwindow* new_window =
      glfwCreateWindow(width, height, title, nullptr, nullptr);
  if (new_window == nullptr) {
    std::println(stderr, "Failed to create GLFW window.");
    glfwTerminate();
    return Window{.valid = false};
  }
  const Window window = {.valid = true,
                         .glfw_window = new_window,
                         .width = width,
                         .height = height};
  return window;
}

void APIENTRY glDebugCallback(GLenum source, GLenum type, GLuint error_id,
                              GLenum severity, GLsizei length,
                              const GLchar* message, const void* userParam) {
  if (severity != GL_DEBUG_SEVERITY_NOTIFICATION) {
    std::println(stderr, "OpenGL debug: {}", message);
  }
}

void glfwErrorCallback(const int error, const char* description) {
  std::println(stderr, "GLFW error {}: {}", error, description);
}

void try_setup_opengl_debug() {
  int context_flags = 0;
  glGetIntegerv(GL_CONTEXT_FLAGS, &context_flags);
  if ((context_flags & GL_CONTEXT_FLAG_DEBUG_BIT) !=
      0) {  // If debug context created
    glEnable(GL_DEBUG_OUTPUT);
    glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
    glDebugMessageCallback(glDebugCallback, nullptr);
    glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr,
                          GL_TRUE);
  }
}

struct FileContent {
  bool valid;
  uint8_t* data;
};

auto read_file_data(const char* filepath) -> FileContent {
  std::ifstream input_stream{filepath, std::ios::binary};
  if (!input_stream.is_open()) {
    return FileContent{.valid = false};
  }
  input_stream.seekg(0, std::ios::end);
  const auto font_file_size = input_stream.tellg();
  input_stream.seekg(0, std::ios::beg);

  auto* data = new uint8_t[font_file_size];
  input_stream.read(reinterpret_cast<char*>(data), font_file_size);
  input_stream.close();

  return FileContent{.valid = true, .data = data};
}

using Program = struct Program {
  bool valid;
  unsigned int program_id;
};

using Shader = struct Shader {
  bool valid;
  unsigned int shader_id;
};

auto compile_shader(const GLenum shader_type, const std::string& source)
    -> Shader {
  const auto shader_id = glCreateShader(shader_type);
  const char* source_ptr = source.c_str();
  glShaderSource(shader_id, 1, &source_ptr, nullptr);
  glCompileShader(shader_id);

  GLint success;
  GLchar infoLog[512];

  glGetShaderiv(shader_id, GL_COMPILE_STATUS, &success);
  if (success == 0) {
    glGetShaderInfoLog(shader_id, 512, nullptr, infoLog);
    std::println(stderr, "Shader Compilation Error: {}", infoLog);
    return Shader{.valid = false};
  }

  return Shader{.valid = true, .shader_id = shader_id};
}

auto compile_and_link_program(const std::string& vertex_shader_source,
                              const std::string& fragment_shader_source)
    -> Program {
  const auto vertex_shader =
      compile_shader(GL_VERTEX_SHADER, vertex_shader_source);
  if (!vertex_shader.valid) {
    return Program{.valid = false};
  }
  const auto fragment_shader =
      compile_shader(GL_FRAGMENT_SHADER, fragment_shader_source);
  if (!fragment_shader.valid) {
    return Program{.valid = false};
  }

  auto program_id = glCreateProgram();
  glAttachShader(program_id, vertex_shader.shader_id);
  glAttachShader(program_id, fragment_shader.shader_id);

  glLinkProgram(program_id);
  int success;
  glGetProgramiv(program_id, GL_LINK_STATUS, &success);
  if (success == 0) {
    GLchar infoLog[512];
    glGetProgramInfoLog(program_id, 512, nullptr, infoLog);
    std::println(stderr, "Program Linking Error: {}", infoLog);
    return Program{.valid = false};
  }
  return Program{.valid = true, .program_id = program_id};
}

using Font = struct Font {
  bool valid;
  uint8_t* texture_data;
  int texture_width;
  int texture_height;
  stbtt_packedchar* packed_chars;
  stbtt_aligned_quad* aligned_quads;
  int codepoint_first_char;
  int glyph_count;
};

// If font_size is too big, the default font atlas image
// size might not fit it.
auto load_font(const char* font_filepath, const int first_unicode_char,
               const int num_chars_in_range, const float font_size) -> Font {
  constexpr size_t width = 1024;
  constexpr size_t height = 1024;

  // uint8_t* font_data;
  const auto font_file_content = read_file_data(font_filepath);
  if (!font_file_content.valid) {
    return Font{.valid = false};
  }

  stbtt_fontinfo font_info{};
  const int fonts_count = stbtt_GetNumberOfFonts(font_file_content.data);
  if (fonts_count <= 0) {
    std::println(stderr, "No valid fonts in file.");
    return Font{.valid = false};
  }
  std::println("{} fonts loaded", fonts_count);

  if (stbtt_InitFont(&font_info, font_file_content.data, 0) == 0) {
    std::println("stbtt_InitFont() failed");
    return Font{.valid = false};
  }
  stbtt_pack_context pack_context;

  auto* font_atlas_bitmap = new uint8_t[width * height];
  if (stbtt_PackBegin(&pack_context, font_atlas_bitmap, width, height, 0, 1,
                      nullptr) == 0) {
    std::println("stbtt_PackBegin failed");
    return Font{.valid = false};
  }

  auto* packed_chars = new stbtt_packedchar[num_chars_in_range];
  stbtt_PackFontRange(&pack_context, font_file_content.data, 0, font_size,
                      first_unicode_char, num_chars_in_range, packed_chars);
  stbtt_PackEnd(&pack_context);
  delete font_file_content.data;

  auto* aligned_quads = new stbtt_aligned_quad[num_chars_in_range];
  for (int i = 0; i < num_chars_in_range; i++) {
    float unused_x;
    float unused_y;

    stbtt_GetPackedQuad(packed_chars, width, height, i, &unused_x, &unused_y,
                        &aligned_quads[i], 0);
  }

  return Font{.valid = true,
              .texture_data = font_atlas_bitmap,
              .texture_width = width,
              .texture_height = height,
              .packed_chars = packed_chars,
              .aligned_quads = aligned_quads,
              .codepoint_first_char = first_unicode_char,
              .glyph_count = num_chars_in_range};
}

auto load_texture(const uint8_t* data, int width, int height) -> unsigned int {
  unsigned int texture_id;
  glGenTextures(1, &texture_id);

  glBindTexture(GL_TEXTURE_2D, texture_id);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

  glTexImage2D(GL_TEXTURE_2D, 0, GL_R8, width, height, 0, GL_RED,
               GL_UNSIGNED_BYTE, data);

  glGenerateMipmap(GL_TEXTURE_2D);
  glBindTexture(GL_TEXTURE_2D, 0);
  return texture_id;
}

using Vertex = struct Vertex {
  glm::vec3 position;
  glm::vec4 color;
  glm::vec2 texture_coordinate;
};

auto get_fullscreen_font_atlas_vertices() -> std::vector<Vertex> {
  std::vector<Vertex> vertices;
  vertices.emplace_back(Vertex{.position = glm::vec3(-1.0F, 1.0F, 0.0F),
                               .color = glm::vec4(1.0F, 1.0F, 1.0F, 1.0F),
                               .texture_coordinate = glm::vec2(0.0F, 0.0F)});
  vertices.emplace_back(Vertex{.position = glm::vec3(1.0F, 1.0F, 0.0F),
                               .color = glm::vec4(1.0F, 1.0F, 1.0F, 1.0F),
                               .texture_coordinate = glm::vec2(1.0F, 0.0F)});
  vertices.emplace_back(Vertex{.position = glm::vec3(-1.0F, -1.0F, 0.0F),
                               .color = glm::vec4(1.0F, 1.0F, 1.0F, 1.0F),
                               .texture_coordinate = glm::vec2(0.0F, 1.0F)});
  vertices.emplace_back(Vertex{.position = glm::vec3(1.0F, -1.0F, 0.0F),
                               .color = glm::vec4(1.0F, 1.0F, 1.0F, 1.0F),
                               .texture_coordinate = glm::vec2(1.0F, 1.0F)});
  return vertices;
}

using Mesh = struct Mesh {
  bool valid;
  unsigned int vao;
  unsigned int vbo;
  unsigned int ebo;
};

auto create_mesh_from_vertices(const std::vector<Vertex>& vertices,
                               const std::vector<unsigned int>& indices)
    -> Mesh {
  unsigned int vao;
  glGenVertexArrays(1, &vao);
  glBindVertexArray(vao);

  unsigned int vbo;
  glGenBuffers(1, &vbo);
  glBindBuffer(GL_ARRAY_BUFFER, vbo);
  glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * vertices.size(),
               vertices.data(), GL_STATIC_DRAW);

  unsigned int ebo;
  glGenBuffers(1, &ebo);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * indices.size(),
               indices.data(), GL_STATIC_DRAW);

  // position attribute:
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 9 * sizeof(float), 0);
  glEnableVertexAttribArray(0);

  // color attribute:
  glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 9 * sizeof(float),
                        (const void*)(3 * sizeof(float)));
  glEnableVertexAttribArray(1);

  // texCoord attribute:
  glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 9 * sizeof(float),
                        (const void*)(7 * sizeof(float)));
  glEnableVertexAttribArray(2);

  glBindVertexArray(0);
  return Mesh{
      .valid = true,
      .vao = vao,
      .vbo = vbo,
      .ebo = ebo,
  };
}

using Text = struct Text {
  bool valid;
  std::vector<Vertex> vertices;
  std::vector<unsigned int> indices;
  Font font;
  unsigned int texture_id;
};

auto DrawText(const std::string& text, const glm::vec3 position,
              const glm::vec4 color, float size, const Font font,
              unsigned int texture_id) -> Text {
  if (!font.valid) {
    return Text{.valid = false};
  }

  float pixel_scale = 2.0F / 600.0F;  // Use window height
  // Crear un punto de inicio usando position
  glm::vec3 cursor_position = position;

  std::vector<Vertex> vertices;
  std::vector<unsigned int> indices;
  int index = 0;
  for (const auto& ch : text) {
    // Leer cada caracter de text, buscar la posicion de su bounding_box
    // dentro de font. Utilizar pixelscale para transformar de un sistema a otro

    // Get glyph

    stbtt_packedchar* packed_char =
        &font.packed_chars[ch - font.codepoint_first_char];
    stbtt_aligned_quad* aligned_quad =
        &font.aligned_quads[ch - font.codepoint_first_char];

    // Calculate glyph size in OpenGL (using pixel_scale)
    glm::vec2 glyph_size = {
        static_cast<float>(packed_char->x1 - packed_char->x0) * pixel_scale *
            size,
        static_cast<float>(packed_char->y1 - packed_char->y0) * pixel_scale *
            size,
    };

    // Determine bounding box bottom left
    glm::vec2 glyph_bounding_box_bottom_left = {
        cursor_position.x + (packed_char->xoff * pixel_scale * size),
        cursor_position.y -
            ((packed_char->yoff + packed_char->y1 - packed_char->y0) *
             pixel_scale * size)};

    // Create vertices
    // Top-right, top-left, bottom-left, bottom-right
    glm::vec2 glyph_vertices[4] = {
        {glyph_bounding_box_bottom_left.x + glyph_size.x,
         glyph_bounding_box_bottom_left.y + glyph_size.y},
        {glyph_bounding_box_bottom_left.x,
         glyph_bounding_box_bottom_left.y + glyph_size.y},
        {glyph_bounding_box_bottom_left.x, glyph_bounding_box_bottom_left.y},
        {glyph_bounding_box_bottom_left.x + glyph_size.x,
         glyph_bounding_box_bottom_left.y},
    };

    // Determine texture coords
    // Top-right, top-left, bottom-left, bottom-right
    glm::vec2 glyph_texture_coordinates[4] = {
        {aligned_quad->s1, aligned_quad->t0},
        {aligned_quad->s0, aligned_quad->t0},
        {aligned_quad->s0, aligned_quad->t1},
        {aligned_quad->s1, aligned_quad->t1},
    };
    indices.emplace_back((index * 4) + 0);
    indices.emplace_back((index * 4) + 1);
    indices.emplace_back((index * 4) + 2);
    indices.emplace_back((index * 4) + 0);
    indices.emplace_back((index * 4) + 2);
    indices.emplace_back((index * 4) + 3);
    index++;

    // Insert vertices
    for (int i = 0; i < 4; i++) {
      vertices.push_back(
          Vertex{.position = glm::vec3(glyph_vertices[i], position.z),
                 .color = color,
                 .texture_coordinate = glyph_texture_coordinates[i]});
    }

    cursor_position.x += packed_char->xadvance * pixel_scale * size;
  }
  return Text{.valid = true,
              .vertices = vertices,
              .indices = indices,
              .font = font,
              .texture_id = texture_id};
}

int main(void) {
  if (glfwInit() == 0) {
    std::println(stderr, "glfw initialization failed");
    return 1;
  }
  Window window =
      initialize_window_with_context(600, 600, "Jonark Text Renderer", true);
  glfwMakeContextCurrent(window.glfw_window);

  if (glewInit() != GLEW_OK) {
    std::println(stderr, "GLEW initialization failed");
    return 1;
  }
  try_setup_opengl_debug();

  static constexpr int first_unicode_char = 32;
  static constexpr int glyph_count = 95;
  const Font font =
      load_font("fonts/arial.ttf", first_unicode_char, glyph_count, 64.0F);
  if (font.valid != true) {
    std::println(stderr, "Could not load font");
    return 1;
  }

  stbi_write_png("font_atlas_mine.png", font.texture_width, font.texture_height,
                 1, font.texture_data, font.texture_width);

  const auto font_atlas_texture_id =
      load_texture(font.texture_data, font.texture_width, font.texture_height);
  if (font_atlas_texture_id == 0) {
    std::println(stderr, "Could not load texture");
    return 1;
  }

  const auto program =
      compile_and_link_program(vertex_shader_source, fragment_shader_source);
  if (program.valid != true) {
    std::println(stderr, "Could not compile and link program");
    return 1;
  }

  float aspect_ratio = 600.0F / 600.0F;
  auto view_projection_matrix =
      glm::ortho(-aspect_ratio, aspect_ratio, -1.0F, 1.0F);

  glUseProgram(program.program_id);

  auto view_projection_uniform_location =
      glGetUniformLocation(program.program_id, "uViewProjectionMat");
  glUniformMatrix4fv(view_projection_uniform_location, 1, GL_FALSE,
                     glm::value_ptr(view_projection_matrix));

  auto font_atlas_texture_uniform_location =
      glGetUniformLocation(program.program_id, "uFontAtlasTexture");
  glUniform1i(font_atlas_texture_uniform_location, 0);

  /* Enable alpha blend for font */
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glBlendEquation(GL_FUNC_ADD);
  glClearColor(0.0F, 0.0F, 0.0F, 0.0F);

  // Draw full font atlas
  auto vertices = get_fullscreen_font_atlas_vertices();
  std::vector<unsigned int> indices = {0, 1, 2, 1, 2, 3};
  auto mesh = create_mesh_from_vertices(vertices, indices);

  // Render
  glBindVertexArray(mesh.vao);

  auto text =
      DrawText("Hola", glm::vec3(-1.0, 0.5, 0.0), glm::vec4(1.0, 1.0, 1.0, 1.0),
               1.0F, font, font_atlas_texture_id);
  auto text_mesh = create_mesh_from_vertices(text.vertices, text.indices);
  glBindVertexArray(text_mesh.vao);
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, text.texture_id);

  while (glfwWindowShouldClose(window.glfw_window) == 0) {
    glClear(GL_COLOR_BUFFER_BIT);

    glDrawElements(GL_TRIANGLES, static_cast<int>(text.indices.size()),
                   GL_UNSIGNED_INT, nullptr);

    glfwPollEvents();
    glfwSwapBuffers(window.glfw_window);
  }
  glDeleteProgram(program.program_id);
  glDeleteTextures(1, &font_atlas_texture_id);

  return 0;
}