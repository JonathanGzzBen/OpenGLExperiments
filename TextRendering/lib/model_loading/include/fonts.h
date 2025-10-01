#ifndef FONTS_H
#define FONTS_H

#include <stb_image_write.h>

#include <fstream>
#include <ios>
#include <string>
#include <vector>

using Vertex2 = struct Vertex2 {
  glm::vec3 position;
  glm::vec4 color;
  glm::vec2 uv;
};

static constexpr uint32_t font_atlas_width = 1024;
static constexpr uint32_t font_atlas_height = 1024;

[[nodiscard]]
inline uint8_t* read_font_atlas(const std::string& filename) {
  auto input_file_stream = std::ifstream(filename, std::ios::binary);
  input_file_stream.seekg(0, std::ios::end);
  const auto size = input_file_stream.tellg();
  input_file_stream.seekg(0, std::ios::beg);

  auto* font_data_buf = new uint8_t[static_cast<size_t>(size)];
  input_file_stream.read(reinterpret_cast<char*>(font_data_buf), size);
  return font_data_buf;
}

uint8_t* load_atlas_bitmap(const uint8_t* font_data_buf) {
  uint8_t* font_atlas_bitmap =
      new uint8_t[font_atlas_width * font_atlas_height];

  // Read from ASCII 32(Space) to ASCII 126(~).
  constexpr uint32_t code_point_first_char = 32;
  constexpr uint32_t amount_chars_to_read = 95;

  // Font pixel height
  constexpr float font_size = 64.0F;

  stbtt_packedchar packed_chars[amount_chars_to_read];
  stbtt_aligned_quad aligned_quads[amount_chars_to_read];

  stbtt_pack_context pack_context;
  stbtt_PackBegin(&pack_context, font_atlas_bitmap, font_atlas_width,
                  font_atlas_height, 0, 1, nullptr);

  stbtt_PackFontRange(
      &pack_context,  // stbtt_pack_context
      font_data_buf,  // Font Atlas texture data
      0,              // Font Index
      font_size,  // Size of font in pixels. (Use STBTT_POINT_SIZE(fontSize) to
                  // use points)
      code_point_first_char,  // Code point of the first character
      amount_chars_to_read,   // No. of charecters to be included in the font
                              // atlas
      packed_chars  // stbtt_packedchar array, this struct will contain the data
                    // to render a glyph
  );
  stbtt_PackEnd(&pack_context);

  for (int i = 0; i < amount_chars_to_read; i++) {
    float unused_x;
    float unused_y;

    stbtt_GetPackedQuad(packed_chars, font_atlas_width, font_atlas_height, i,
                        &unused_x, &unused_y, &aligned_quads[i], 0);
  }

  stbi_write_png("fontAtlas.png", font_atlas_width, font_atlas_height, 1,
                 font_atlas_bitmap, font_atlas_width);
  return font_atlas_bitmap;
}

unsigned int generate_font_atlas_texture(const uint8_t* font_atlas_bitmap) {
  if (font_atlas_bitmap == nullptr) {
    return 0;
  }
  uint32_t font_atlas_texture_id;
  glGenTextures(1, &font_atlas_texture_id);
  glBindTexture(GL_TEXTURE_2D, font_atlas_texture_id);

  glTexImage2D(GL_TEXTURE_2D, 0, GL_R8, font_atlas_width, font_atlas_height, 0,
               GL_RED, GL_UNSIGNED_BYTE, font_atlas_bitmap);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

  glBindTexture(GL_TEXTURE_2D, 0);
  return font_atlas_texture_id;
}

void draw_text(const std::string& text, glm::vec3 position, glm::vec4 color,
               float size, float pixel_scale, stbtt_packedchar packed_chars[],
               stbtt_aligned_quad aligned_quads[],
               unsigned int code_point_first_char,
               unsigned int chars_to_include_in_font_alias) {
  glm::vec3 localPosition = position;

  std::vector<Vertex2> vertices;
  for (char ch : text) {
    if (ch >= code_point_first_char &&
        ch <= code_point_first_char + chars_to_include_in_font_alias) {
      stbtt_packedchar* packed_char = &packed_chars[ch - code_point_first_char];
      stbtt_aligned_quad* aligned_quad =
          &aligned_quads[ch - code_point_first_char];

      glm::vec2 glyph_size = {
          (packed_char->x1 - packed_char->x0) * pixel_scale * size,
          (packed_char->y1 - packed_char->y0) * pixel_scale * size,
      };

      glm::vec2 glyph_bounding_box_bottom_left = {
          localPosition.x + (packed_char->xoff * pixel_scale * size),
          localPosition.y +
              (packed_char->yoff + packed_char->y1 - packed_char->y0) *
                  pixel_scale * size,
      };

      glm::vec2 glyph_vertices[4] = {
          {glyph_bounding_box_bottom_left.x + glyph_size.x,
           glyph_bounding_box_bottom_left.y + glyph_size.y},
          {glyph_bounding_box_bottom_left.x,
           glyph_bounding_box_bottom_left.y + glyph_size.y},
          {glyph_bounding_box_bottom_left.x, glyph_bounding_box_bottom_left.y},
          {glyph_bounding_box_bottom_left.x + glyph_size.x,
           glyph_bounding_box_bottom_left.y},
      };

      glm::vec2 glyph_texture_coordinates[4] = {
          {aligned_quad->s1, aligned_quad->t0},
          {aligned_quad->s0, aligned_quad->t0},
          {aligned_quad->s0, aligned_quad->t1},
          {aligned_quad->s1, aligned_quad->t1}};

      // int vertex_index = 0; Tal vez deba ser local, sobretodo si uso un mesh
      static int vertex_index = 0;
      int order[6] = {0, 1, 2, 0, 2, 3};
      for (int i = 0; i < 6; i++) {
        vertices[vertex_index + i].position =
            glm::vec3(glyph_vertices[order[i]], position.z);
        vertices[vertex_index + i].color = color;
        vertices[vertex_index + i].uv = glyph_texture_coordinates[order[i]];
      }
      vertex_index += 6;

      localPosition.x += packed_char->xadvance * pixel_scale * size;
    } else if (ch == '\n') {                          // Handle newline
      localPosition.y -= 64.0f * pixel_scale * size;  // font_size
      localPosition.x = position.x;
    }
  }
}

#endif  // FONTS_H
