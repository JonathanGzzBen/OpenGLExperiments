#ifndef TEXT_H
#define TEXT_H

#include <stb_image_write.h>
#include <stb_truetype.h>

#include <print>

struct TextManager {};

struct Font {
  bool valid;
  uint8_t* bitmap;
  std::unique_ptr<stbtt_packedchar[]> packed_chars;
  std::unique_ptr<stbtt_aligned_quad[]> aligned_quads;
  int charcode_begin;
  int charcode_count;
};

inline auto text_get_mesh(const Font& font, MeshManager* manager,
                          const glm::vec2 position, const std::string& text,
                          const float size, const float pixel_scale,
                          const unsigned int font_atlas_texture_id)
    -> MeshHandle {
  // Process text into vertices
  glm::vec2 cursor_position = position;
  const unsigned int num_vertices = text.length() * 4;
  auto* vertices = new Vertex[num_vertices];
  size_t vertices_index = 0;
  auto* indices = new unsigned int[text.length() * 6];
  size_t indices_index = 0;
  for (const auto character : text) {
    const int index = character - font.charcode_begin;
    if (index < 0 || font.charcode_count <= index) {
      std::println(stderr, "Could not find character {}", character);
      return -1;
    }
    const auto packed_char = font.packed_chars[index];
    const auto aligned_quad = font.aligned_quads[index];

    const auto glyph_size =
        glm::vec2(static_cast<float>(packed_char.x1 - packed_char.x0) *
                      pixel_scale * size,
                  static_cast<float>(packed_char.y1 - packed_char.y0) *
                      pixel_scale * size);

    const auto bounding_box_top_left_position =
        glm::vec2(cursor_position.x + (packed_char.xoff * pixel_scale * size),
                  cursor_position.y +
                      ((packed_char.yoff + packed_char.y1 - packed_char.y0) *
                       pixel_scale * size));

    // Top-right, top-left, bottom-left, bottom-right
    const Vertex character_vertices[4] = {
        {.position =
             glm::vec3(bounding_box_top_left_position.x + glyph_size.x,
                       bounding_box_top_left_position.y + glyph_size.y, 0.0F),
         .uv = glm::vec2(aligned_quad.s1, aligned_quad.t0)},
        {.position =
             glm::vec3(bounding_box_top_left_position.x,
                       bounding_box_top_left_position.y + glyph_size.y, 0.0F),
         .uv = glm::vec2(aligned_quad.s0, aligned_quad.t0)},
        {.position = glm::vec3(bounding_box_top_left_position.x,
                               bounding_box_top_left_position.y, 0.0F),
         .uv = glm::vec2(aligned_quad.s0, aligned_quad.t1)},
        {.position = glm::vec3(bounding_box_top_left_position.x + glyph_size.x,
                               bounding_box_top_left_position.y, 0.0F),
         .uv = glm::vec2(aligned_quad.s1, aligned_quad.t1)},
    };

    vertices[vertices_index++] = character_vertices[0];
    vertices[vertices_index++] = character_vertices[1];
    vertices[vertices_index++] = character_vertices[2];
    vertices[vertices_index++] = character_vertices[3];

    unsigned int base = vertices_index - 4;  // We just added 4 vertices
    indices[indices_index++] = base + 0;
    indices[indices_index++] = base + 1;
    indices[indices_index++] = base + 2;
    indices[indices_index++] = base + 0;
    indices[indices_index++] = base + 2;
    indices[indices_index++] = base + 3;
    cursor_position.x += packed_char.xadvance * pixel_scale * size;
  }

  const auto mesh_data = MeshData{
      .valid = true,
      .vertices = vertices,
      .num_vertices = num_vertices,
      .indices = indices,
      .num_indices = 6 * text.length(),
      .texture_id = font_atlas_texture_id,
  };
  const auto mesh_handle = mesh_create(*manager, mesh_data);
  delete vertices;
  delete indices;
  if (mesh_handle < 0) {
    std::println("Could not create mesh for text");
    return -1;
  }
  return mesh_handle;
}

inline auto load_font(const unsigned char* font_binary_data,
                      const int charcode_begin, const int charcode_count,
                      const float font_size, const int font_atlas_width,
                      const int font_atlas_height) {
  const auto number_of_fonts = stbtt_GetNumberOfFonts(font_binary_data);
  std::println("Read {} fonts", number_of_fonts);
  if (number_of_fonts != 1) {
    std::println(stderr, "File doesn't have 1 font.");
    return Font{.valid = false};
  }

  stbtt_pack_context pack_context;
  auto* bitmap =
      new uint8_t[static_cast<size_t>(font_atlas_width * font_atlas_height)];
  if (stbtt_PackBegin(&pack_context, bitmap, font_atlas_width,
                      font_atlas_height, 0, 1, nullptr) != 1) {
    std::println(stderr, "Could not initialize font packing context");
    return Font{.valid = false};
  }

  auto packed_chars = std::make_unique<stbtt_packedchar[]>(charcode_count);
  stbtt_PackFontRange(&pack_context, font_binary_data, 0, font_size,
                      charcode_begin, charcode_count, packed_chars.get());

  auto aligned_quads = std::make_unique<stbtt_aligned_quad[]>(charcode_count);

  for (int i = 0; i < charcode_count; ++i) {
    static float x_unused;
    static float y_unused;
    stbtt_GetPackedQuad(packed_chars.get(), font_atlas_width, font_atlas_height,
                        i, &x_unused, &y_unused, &aligned_quads[i], 0);
  }

  stbi_write_png("font_atlas.png", font_atlas_width, font_atlas_height, 1,
                 bitmap, font_atlas_width);

  stbtt_PackEnd(&pack_context);

  return Font{.valid = true,
              .bitmap = bitmap,
              .packed_chars = std::move(packed_chars),
              .aligned_quads = std::move(aligned_quads),
              .charcode_begin = charcode_begin,
              .charcode_count = charcode_count};
}

#endif  // TEXT_H
