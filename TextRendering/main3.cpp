#include <string>
#define GLAD_GL_IMPLEMENTATION
// clang-format off
#include <glad/glad.h>
#include <GLFW/glfw3.h>  // This must go aafter glad.h
// clang-format on

#define STB_TRUETYPE_IMPLEMENTATION
#include <stb_truetype.h>
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image_write.h>

#include <fstream>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <ios>
#include <vector>

// #include "lib/model_loading/include/fonts.h"

std::string vertexShaderSrc = R"(
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

std::string fragmentShaderSrc = R"(
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

struct Vertex {
  glm::vec3 position;
  glm::vec4 color;
  glm::vec2 texCoord;
};

const size_t VBO_SIZE = 600000 * sizeof(Vertex);
static constexpr uint32_t font_atlas_width = 1024;
static constexpr uint32_t font_atlas_height = 1024;

typedef struct GlobalState {
  int current_window_height;
  glm::mat4 view_projection_matrix;
  unsigned int vertex_index;
} GlobalState;

GlobalState global_state;

[[nodiscard]]
static unsigned int SetupShaderProgram(const char* vertexShaderSource,
                                       const char* fragmentShaderSource) {
  uint32_t vtxShaderID, fragShaderID;

  // Vertex Shader: -----------------------------------
  vtxShaderID = glCreateShader(GL_VERTEX_SHADER);
  glShaderSource(vtxShaderID, 1, &vertexShaderSource, nullptr);
  glCompileShader(vtxShaderID);

  // Fragment Shader: ---------------------------------
  fragShaderID = glCreateShader(GL_FRAGMENT_SHADER);
  glShaderSource(fragShaderID, 1, &fragmentShaderSource, nullptr);
  glCompileShader(fragShaderID);

  // Checking for shader compilation errors: -------------------
  GLint success;
  GLchar infoLog[512];

  glGetShaderiv(vtxShaderID, GL_COMPILE_STATUS, &success);
  if (!success) {
    glGetShaderInfoLog(vtxShaderID, 512, NULL, infoLog);
    fprintf(stderr, "Vertex Shader Compilation Error: %s\n", infoLog);
    return 0;
  }

  glGetShaderiv(fragShaderID, GL_COMPILE_STATUS, &success);
  if (!success) {
    glGetShaderInfoLog(fragShaderID, 512, NULL, infoLog);
    fprintf(stderr, "Fragment Shader Compilation Error: %s\n", infoLog);
    return 0;
  }

  // Linking the shaders into a shader program: ----------------------
  unsigned int program_id = glCreateProgram();
  glAttachShader(program_id, vtxShaderID);
  glAttachShader(program_id, fragShaderID);

  glLinkProgram(program_id);

  // Check for linking errors: ---------------------------------------
  glGetProgramiv(program_id, GL_LINK_STATUS, &success);
  if (!success) {
    glGetProgramInfoLog(program_id, 512, NULL, infoLog);
    fprintf(stderr, "Program Linking Error: %s\n", infoLog);
    return 0;
  }
  return program_id;
}
static void SetupViewProjection(float aspectRatio) {
  glm::mat4 projectionMat = glm::ortho(-aspectRatio, aspectRatio, -1.0f, 1.0f);
  glm::mat4 viewMat = glm::mat4(1.0f);

  viewMat = glm::translate(viewMat, {0.0f, 0.0f, 0.0f});
  viewMat = glm::rotate(viewMat, 0.0f, {1, 0, 0});
  viewMat = glm::rotate(viewMat, 0.0f, {0, 1, 0});
  viewMat = glm::rotate(viewMat, 0.0f, {0, 0, 1});
  viewMat = glm::scale(viewMat, {1.0f, 1.0f, 1.0f});

  global_state.view_projection_matrix = projectionMat * viewMat;
}

static GLFWwindow* SetupWindowAndContext(uint32_t windowWidth,
                                         uint32_t windowHeight,
                                         const std::string& title) {
  // Initialize the window:
  if (!glfwInit()) {
    fprintf(stderr, "GLFW Failed to initialize!\n");
    exit(-1);
  }

  // Create the window

  GLFWwindow* window = glfwCreateWindow(windowWidth, windowHeight,
                                        title.c_str(), nullptr, nullptr);
  glfwMakeContextCurrent(window);

  gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);

  // int version = gladLoadGL(glfwGetProcAddress);
  // std::cout << "Loaded: OpenGL " << GLAD_VERSION_MAJOR(version) << "." <<
  // GLAD_VERSION_MINOR(version);

  // Setup View Projection matrix:
  SetupViewProjection((float)windowWidth / (float)windowHeight);
  global_state.current_window_height = windowHeight;

  glfwSetWindowSizeCallback(window,
                            [](GLFWwindow* window, int width, int height) {
                              SetupViewProjection((float)width / (float)height);
                              global_state.current_window_height = height;

                              // Resize the viewport
                              glViewport(0, 0, width, height);
                            });

  // Setup Alpha blending.
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glBlendEquation(GL_FUNC_ADD);
  return window;
}

static int SetupVaoAndVbo(unsigned int* out_vao, unsigned int* out_vbo) {
  unsigned int vbo;
  glGenBuffers(1, &vbo);
  glBindBuffer(GL_ARRAY_BUFFER, vbo);
  glBufferData(GL_ARRAY_BUFFER, VBO_SIZE, nullptr, GL_DYNAMIC_DRAW);

  unsigned int vao;
  glGenVertexArrays(1, &vao);
  glBindVertexArray(vao);

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

  *out_vao = vao;
  *out_vbo = vbo;
  return 0;  // Success
}

static uint8_t* SetupFont(const std::string& font_file, const float font_size,
                          const uint32_t code_point_first_char,
                          const uint32_t glyph_count,
                          stbtt_packedchar* packed_chars,
                          stbtt_aligned_quad* aligned_quads) {
  std::ifstream input_stream(font_file, std::ios::binary);
  if (!input_stream) {
    fprintf(stderr, "Failed to open font file!\n");
    return nullptr;
  }
  input_stream.seekg(0, std::ios::end);
  const size_t font_file_size = input_stream.tellg();
  input_stream.seekg(0, std::ios::beg);

  uint8_t* font_data_buf = new uint8_t[font_file_size];
  input_stream.read((char*)font_data_buf, font_file_size);

  stbtt_fontinfo font_info = {};

  uint32_t font_count = stbtt_GetNumberOfFonts(font_data_buf);
  printf("Font file %s has %u fonts\n", font_file.c_str(), font_count);

  if (!stbtt_InitFont(&font_info, font_data_buf, 0)) {
    fprintf(stderr, "Failed to initialize font!\n");
    return nullptr;
  }

  uint8_t* font_atlas_texture_data =
      new uint8_t[font_atlas_width * font_atlas_height];

  stbtt_pack_context pack_context;

  stbtt_PackBegin(&pack_context, font_atlas_texture_data, font_atlas_width,
                  font_atlas_height, 0, 1, nullptr);

  stbtt_PackFontRange(&pack_context, font_data_buf, 0, font_size,
                      code_point_first_char, glyph_count, packed_chars);
  stbtt_PackEnd(&pack_context);

  // stbtt_aligned_quad aligned_quads[glyph_count];
  for (int i = 0; i < glyph_count; i++) {
    // Unused variables because I will be using
    // OpenGL coordinate system.
    float unused_x;
    float unused_y;

    stbtt_GetPackedQuad(packed_chars, font_atlas_width, font_atlas_height, i,
                        &unused_x, &unused_y, &aligned_quads[i], 0);
  }

  delete[] font_data_buf;

  stbi_write_png("font_atlas.png", font_atlas_width, font_atlas_height, 1,
                 font_atlas_texture_data, font_atlas_width);
  return font_atlas_texture_data;
}

static unsigned int SetupFontTexture(const uint32_t texture_width,
                                     const uint32_t texture_height,
                                     void* texture_bitmap) {
  unsigned int texture_id;
  glGenTextures(1, &texture_id);

  glBindTexture(GL_TEXTURE_2D, texture_id);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

  glTexImage2D(GL_TEXTURE_2D, 0, GL_R8, texture_width, texture_height, 0,
               GL_RED, GL_UNSIGNED_BYTE, texture_bitmap);

  glBindTexture(GL_TEXTURE_2D, 0);
  return texture_id;
}

static void Render(const unsigned int program_id, const unsigned int vao,
                   const unsigned int vbo, glm::mat4 view_projection_matrix,
                   const std::vector<Vertex>& vertices) {
  size_t sizeOfVertices = vertices.size() * sizeof(Vertex);
  uint32_t drawCallCount =
      (sizeOfVertices / VBO_SIZE) + 1;  // aka number of chunks.

  // Render each chunk of vertex data.
  for (int i = 0; i < drawCallCount; i++) {
    const Vertex* data = vertices.data() + i * VBO_SIZE;

    uint32_t vertexCount = i == drawCallCount - 1
                               ? (sizeOfVertices % VBO_SIZE) / sizeof(Vertex)
                               : VBO_SIZE / (sizeof(Vertex) * 6);

    int uniformLocation =
        glGetUniformLocation(program_id, "uViewProjectionMat");
    glUniformMatrix4fv(uniformLocation, 1, GL_TRUE,
                       glm::value_ptr(view_projection_matrix));

    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferSubData(
        GL_ARRAY_BUFFER, 0,
        i == drawCallCount - 1 ? sizeOfVertices % VBO_SIZE : VBO_SIZE, data);

    glDrawArrays(GL_TRIANGLES, 0, vertexCount);
  }
}

glm::mat4 GetViewProjectionMatrix(float aspectRatio) {
  glm::mat4 projectionMat = glm::ortho(-aspectRatio, aspectRatio, -1.0f, 1.0f);
  glm::mat4 viewMat = glm::mat4(1.0f);

  viewMat = glm::translate(viewMat, {0.0f, 0.0f, 0.0f});
  viewMat = glm::rotate(viewMat, 0.0f, {1, 0, 0});
  viewMat = glm::rotate(viewMat, 0.0f, {0, 1, 0});
  viewMat = glm::rotate(viewMat, 0.0f, {0, 0, 1});
  viewMat = glm::scale(viewMat, {1.0f, 1.0f, 1.0f});

  return projectionMat * viewMat;
}

static void DrawText(const std::string& text, glm::vec3 position,
                     glm::vec4 color, float size, float window_height,
                     uint32_t code_point_first_char, uint32_t glyph_count,
                     stbtt_packedchar* packed_chars,
                     stbtt_aligned_quad* aligned_quads,
                     std::vector<Vertex>& vertices) {
  int order[6] = {0, 1, 2, 0, 2, 3};
  // Convert from pixel to Normalized Device Coordinates (NDC)
  // OpenGL ranges from -1 to +1, hence the 2.0 to get the size of 1 pixel
  float pixel_scale = 2.0F / window_height;

  glm::vec3 local_position = position;

  for (char ch : text) {
            // Check if the charecter glyph is in the font atlas.
        if(ch >= code_point_first_char && ch <= code_point_first_char + glyph_count)
        {
            if(vertices.size() <= global_state.vertex_index)
                vertices.resize(vertices.size() + 6);

            // Retrive the data that is used to render a glyph of charecter 'ch'
            stbtt_packedchar* packedChar = &packed_chars[ch - code_point_first_char];
            stbtt_aligned_quad* alignedQuad = &aligned_quads[ch - code_point_first_char];

            // The units of the fields of the above structs are in pixels,
            // convert them to a unit of what we want be multilplying to pixelScale
            glm::vec2 glyphSize =
            {
                (packedChar->x1 - packedChar->x0) * pixel_scale * size,
                (packedChar->y1 - packedChar->y0) * pixel_scale * size
            };

            glm::vec2 glyphBoundingBoxBottomLeft =
            {
                local_position.x + (packedChar->xoff * pixel_scale * size),
                local_position.y - (packedChar->yoff + packedChar->y1 - packedChar->y0) * pixel_scale * size
            };

            // The order of vertices of a quad goes top-right, top-left, bottom-left, bottom-right
            glm::vec2 glyphVertices[4] =
            {
                { glyphBoundingBoxBottomLeft.x + glyphSize.x, glyphBoundingBoxBottomLeft.y + glyphSize.y },
                { glyphBoundingBoxBottomLeft.x, glyphBoundingBoxBottomLeft.y + glyphSize.y },
                { glyphBoundingBoxBottomLeft.x, glyphBoundingBoxBottomLeft.y },
                { glyphBoundingBoxBottomLeft.x + glyphSize.x, glyphBoundingBoxBottomLeft.y },
            };

            glm::vec2 glyphTextureCoords[4] =
            {
                { alignedQuad->s1, alignedQuad->t0 },
                { alignedQuad->s0, alignedQuad->t0 },
                { alignedQuad->s0, alignedQuad->t1 },
                { alignedQuad->s1, alignedQuad->t1 },
            };

            // We need to fill the vertex buffer by 6 vertices to render a quad as we are rendering a quad as 2 triangles
            // The order used is in the 'order' array
            // order = [0, 1, 2, 0, 2, 3] is meant to represent 2 triangles:
            // one by glyphVertices[0], glyphVertices[1], glyphVertices[2] and one by glyphVertices[0], glyphVertices[2], glyphVertices[3]
            for(int i = 0; i < 6; i++)
            {
                vertices[global_state.vertex_index + i].position = glm::vec3(glyphVertices[order[i]], position.z);
                vertices[global_state.vertex_index + i].color = color;
                vertices[global_state.vertex_index + i].texCoord = glyphTextureCoords[order[i]];
            }

            global_state.vertex_index += 6;

            // Update the position to render the next glyph specified by packedChar->xadvance.
            local_position.x += packedChar->xadvance * pixel_scale * size;
        }

        // Handle newlines seperately.
        else if(ch == '\n')
        {
            // advance y by fontSize, reset x-coordinate
            local_position.y -= 64.0F * pixel_scale * size;
            local_position.x = position.x;
        }
  }
}

auto main() -> int {
  GLFWwindow* window = SetupWindowAndContext(600, 600, "Jonark Text Renderer");
  const unsigned int program_id =
      SetupShaderProgram(vertexShaderSrc.c_str(), fragmentShaderSrc.c_str());

  unsigned int vao = 0;
  unsigned int vbo = 0;
  if (SetupVaoAndVbo(&vao, &vbo) != 0) {
    fprintf(stderr, "SetupVaoAndVbo() failed!\n");
    return -1;
  }

  // Load Font Texture
  static constexpr float font_size = 64.0f;
  constexpr uint32_t code_point_first_char = 32;
  constexpr uint32_t glyph_count = 95;
  stbtt_packedchar packed_chars[glyph_count];
  stbtt_aligned_quad aligned_quads[glyph_count];
  uint8_t* font_texture_data =
      SetupFont("fonts/arial.ttf", font_size, code_point_first_char,
                glyph_count, packed_chars, aligned_quads);

  // SetupFontTexture...
  const unsigned int texture_id =
      SetupFontTexture(font_atlas_width, font_atlas_height, font_texture_data);

  delete[] font_texture_data;
  glUseProgram(program_id);

  std::vector<Vertex> vertices;

  auto view_projection_matrix = GetViewProjectionMatrix(600.0F / 600.0F);
  glClearColor(0.0F, 0.0F, 0.0F, 1.0F);
  while (glfwWindowShouldClose(window) == 0) {
    glClear(GL_COLOR_BUFFER_BIT);

    global_state.vertex_index = 0;  // Reset vertex

    // Draw
    glBindTexture(GL_TEXTURE_2D, texture_id);
    glActiveTexture(GL_TEXTURE0);
    const int texture_uniform_location_ =
        glGetUniformLocation(program_id, "uFontAtlasTexture");
    glUniform1i(texture_uniform_location_, 0);

    DrawText("stb_truetype.h example", {-0.8f, 0.4f, 0.0f},
             {0.9f, 0.2f, 0.3f, 1.0f}, 1.0f, 600.0F, code_point_first_char,
             glyph_count, packed_chars, aligned_quads, vertices);

    Render(program_id, vao, vbo, view_projection_matrix, vertices);

    glfwPollEvents();
    glfwSwapBuffers(window);
  }

  return 0;
}