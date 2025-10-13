#ifndef TEXT_H
#define TEXT_H

#include <stb_image_write.h>

#include <print>

#include "jtr/font.h"

inline auto text_create_mesh(const Font& font, MeshManager* manager,
                             const glm::vec2 position, const std::string& text,
                             const float size, const float pixel_scale,
                             const unsigned int font_atlas_texture_id)
    -> MeshHandle {
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

#endif  // TEXT_H
