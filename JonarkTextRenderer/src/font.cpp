#include "jtr/font.h"

#include <iostream>
#include <print>

auto font_manager_create(const int max_num_fonts) -> FontManager {
  if (max_num_fonts <= 0) {
    std::println(std::cerr, "Invalid max number of fonts");
    return FontManager{.valid = false};
  }
  return FontManager{
      .valid = true,
      .fonts_count = 0,
      .max_num_fonts = max_num_fonts,
      .bitmaps = new uint8_t*[max_num_fonts],
      .packed_chars_s = new stbtt_packedchar*[max_num_fonts],
      .aligned_quads_s = new stbtt_aligned_quad*[max_num_fonts],
      .charcode_begins = new int[max_num_fonts],
      .charcode_counts = new int[max_num_fonts],
  };
}

auto font_manager_destroy_all(FontManager* const manager) -> void {
  if (manager == nullptr || !manager->valid) {
    std::println(std::cerr, "Invalid font manager");
    return;
  }

  for (FontHandle handle = 0; handle < manager->fonts_count; ++handle) {
    font_destroy(manager, handle);
  }

  manager->valid = false;
  manager->fonts_count = 0;
  manager->max_num_fonts = 0;
  delete manager->bitmaps;
  delete manager->packed_chars_s;
  delete manager->aligned_quads_s;
  delete manager->charcode_begins;
  delete manager->charcode_counts;
}

auto font_validate_handle(const FontManager& manager, const FontHandle handle)
    -> bool {
  if (!manager.valid) {
    std::println(std::cerr, "Invalid font manager");
    return false;
  }
  if (handle < 0 || manager.fonts_count <= handle) {
    std::println(std::cerr, "Invalid font handle");
    return false;
  }
  if (manager.bitmaps[handle] == nullptr) {
    std::println(std::cerr, "Font for handle not valid");
    return false;
  }
  return true;
}

auto font_get_data(const FontManager& manager, const FontHandle handle)
    -> FontData {
  if (!font_validate_handle(manager, handle)) {
    std::println(stderr, "Invalid font handle");
    return FontData{.valid = false};
  }
  return FontData{.valid = true,
                  .packed_chars = manager.packed_chars_s[handle],
                  .aligned_quads = manager.aligned_quads_s[handle],
                  .charcode_begin = manager.charcode_begins[handle],
                  .charcode_count = manager.charcode_counts[handle]};
}

auto font_create(FontManager* const manager,
                 const unsigned char* font_binary_data,
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

  const FontHandle handle = manager->fonts_count++;
  manager->bitmaps[handle] = bitmap;
  manager->packed_chars_s[handle] = packed_chars;
  manager->aligned_quads_s[handle] = aligned_quads;
  manager->charcode_begins[handle] = charcode_begin;
  manager->charcode_counts[handle] = charcode_count;
  return handle;
}

auto font_destroy(const FontManager* manager, const FontHandle handle) -> void {
  if (!font_validate_handle(*manager, handle)) {
    std::println(std::cerr, "Invalid font handle");
    return;
  }

  delete manager->bitmaps[handle];
  manager->bitmaps[handle] = nullptr;
  delete manager->packed_chars_s[handle];
  manager->packed_chars_s[handle] = nullptr;
  delete manager->aligned_quads_s[handle];
  manager->aligned_quads_s[handle] = nullptr;
  manager->charcode_begins[handle] = 0;
  manager->charcode_counts[handle] = 0;
}
