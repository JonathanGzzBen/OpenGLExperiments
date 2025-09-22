#ifndef FONTS_H
#define FONTS_H

#include <stb_image_write.h>

#include <fstream>
#include <ios>
#include <string>

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

void load_atlas_bitmap(const uint8_t* font_data_buf) {
  constexpr uint32_t font_atlas_width = 1024;
  constexpr uint32_t font_atlas_height = 1024;

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
}

#endif  // FONTS_H
