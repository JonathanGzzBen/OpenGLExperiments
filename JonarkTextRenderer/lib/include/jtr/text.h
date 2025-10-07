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

inline auto load_font(const unsigned char* font_binary_data,
                      const int charcode_begin, const int charcode_count,
                      const float font_size, const int width, const int height,
                      const int font_atlas_width, const int font_atlas_height) {
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

  float x_unused;
  float y_unused;
  auto aligned_quads = std::make_unique<stbtt_aligned_quad[]>(charcode_count);

  for (int i = 0; i < charcode_count; ++i) {
    stbtt_GetPackedQuad(&packed_chars[i], width, height, i, &x_unused,
                        &y_unused, &aligned_quads[i], 0);
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
