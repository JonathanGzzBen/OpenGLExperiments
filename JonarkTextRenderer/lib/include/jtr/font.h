#ifndef FONT_H
#define FONT_H

#include <stb_truetype.h>

#include <cstdint>

struct Font {
  bool valid;
  uint8_t* bitmap;
  stbtt_packedchar* packed_chars;
  stbtt_aligned_quad* aligned_quads;
  int charcode_begin;
  int charcode_count;
};

using FontHandle = int;

struct FontManager {
  bool valid;
  Font* fonts;
  int fonts_count;
  int max_num_fonts;
};

auto font_manager_create(int max_num_fonts) -> FontManager;

auto font_manager_destroy_all(FontManager* manager) -> void;

auto font_get(const FontManager& manager, FontHandle handle) -> Font*;

auto font_create(FontManager* manager, const unsigned char* font_binary_data,
                 int charcode_begin, int charcode_count, float font_size,
                 int font_atlas_width, int font_atlas_height) -> FontHandle;

auto font_destroy(const FontManager* manager, FontHandle handle) -> void;

#endif  // FONT_H
