#ifndef FONT_H
#define FONT_H

#include <stb_truetype.h>

#include <cstdint>

using FontHandle = int;

struct FontData {
  const bool valid;
  const stbtt_packedchar* const packed_chars;
  const stbtt_aligned_quad* const aligned_quads;
  const int charcode_begin;
  const int charcode_count;
};

struct FontManager {
  bool valid;
  int fonts_count;
  int max_num_fonts;

  uint8_t** bitmaps;
  stbtt_packedchar** packed_chars_s;
  stbtt_aligned_quad** aligned_quads_s;
  int* charcode_begins;
  int* charcode_counts;
};

auto font_manager_create(int max_num_fonts) -> FontManager;

auto font_manager_destroy_all(FontManager* manager) -> void;

auto font_validate_handle(const FontManager& manager, FontHandle handle)
    -> bool;

auto font_get_data(const FontManager& manager, FontHandle handle) -> FontData;

auto font_create(FontManager* manager, const unsigned char* font_binary_data,
                 int charcode_begin, int charcode_count, float font_size,
                 int font_atlas_width, int font_atlas_height) -> FontHandle;

auto font_destroy(const FontManager* manager, FontHandle handle) -> void;

#endif  // FONT_H
