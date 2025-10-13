#include "jtr/font.h"

#include <iostream>
#include <print>

auto font_manager_create(const int max_num_fonts) -> FontManager {
  if (max_num_fonts <= 0) {
    std::println(std::cerr, "Invalid max number of fonts");
    return FontManager{.valid = false};
  }
  auto* const fonts_ptr = new Font[max_num_fonts];
  return FontManager{.valid = true,
                     .fonts = fonts_ptr,
                     .fonts_count = 0,
                     .max_num_fonts = max_num_fonts};
}

auto font_manager_destroy_all(FontManager* manager) -> void {
  if (manager == nullptr || !manager->valid) {
    std::println(std::cerr, "Invalid font manager");
    return;
  }

  for (FontHandle handle = 0; handle < manager->fonts_count; ++handle) {
    font_destroy(manager, handle);
  }

  manager->valid = false;
  manager->fonts = nullptr;
  manager->fonts_count = 0;
  manager->max_num_fonts = 0;
}

auto font_get(const FontManager& manager, FontHandle handle) -> Font* {
  if (!manager.valid) {
    std::println(std::cerr, "Invalid font manager");
    return nullptr;
  }
  if (handle < 0 || manager.fonts_count <= handle) {
    std::println(std::cerr, "Invalid font handle");
    return nullptr;
  }
  if (!manager.fonts[handle].valid) {
    std::println(std::cerr, "Font for handle not valid");
    return nullptr;
  }
  return &manager.fonts[handle];
}

auto font_create(FontManager* manager, const unsigned char* font_binary_data,
                 const int charcode_begin, const int charcode_count,
                 const float font_size, const int font_atlas_width,
                 const int font_atlas_height) -> FontHandle {
  const auto number_of_fonts = stbtt_GetNumberOfFonts(font_binary_data);
  std::println("Read {} fonts", number_of_fonts);
  if (number_of_fonts != 1) {
    std::println(stderr, "File doesn't have 1 font.");
    return -1;
  }

  stbtt_pack_context pack_context;
  auto* bitmap =
      new uint8_t[static_cast<size_t>(font_atlas_width * font_atlas_height)];
  if (stbtt_PackBegin(&pack_context, bitmap, font_atlas_width,
                      font_atlas_height, 0, 1, nullptr) != 1) {
    std::println(stderr, "Could not initialize font packing context");
    return -1;
  }

  auto* packed_chars = new stbtt_packedchar[charcode_count];
  stbtt_PackFontRange(&pack_context, font_binary_data, 0, font_size,
                      charcode_begin, charcode_count, packed_chars);

  auto* aligned_quads = new stbtt_aligned_quad[charcode_count];

  for (int i = 0; i < charcode_count; ++i) {
    static float x_unused;
    static float y_unused;
    stbtt_GetPackedQuad(packed_chars, font_atlas_width, font_atlas_height, i,
                        &x_unused, &y_unused, &aligned_quads[i], 0);
  }

  stbtt_PackEnd(&pack_context);

  const auto new_font = Font{.valid = true,
                             .bitmap = bitmap,
                             .packed_chars = packed_chars,
                             .aligned_quads = aligned_quads,
                             .charcode_begin = charcode_begin,
                             .charcode_count = charcode_count};
  const auto handle = manager->fonts_count++;  // Increase count
  manager->fonts[handle] = new_font;
  return handle;
}

auto font_destroy(const FontManager* manager, const FontHandle handle) -> void {
  auto* const font = font_get(*manager, handle);
  if (font == nullptr) {
    std::println(std::cerr, "Invalid font handle");
    return;
  }

  delete font->packed_chars;
  delete font->aligned_quads;
  font->valid = false;
}
