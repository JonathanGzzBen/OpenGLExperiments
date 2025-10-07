#ifndef TEXTURE_H
#define TEXTURE_H
#include <GL/glew.h>

#include <iostream>
#include <print>

using Texture = struct Texture {
  bool valid;
  unsigned int texture_id;
};

using TextureHandle = int;

using TextureManager = struct TextureManager {
  bool valid;
  Texture* textures;
  int num_textures;
  int max_num_textures;
};

auto texture_manager_create(int max_num_textures) -> TextureManager;

auto texture_manager_destroy_all(TextureManager* manager) -> void;

auto texture_get(const TextureManager& manager, TextureHandle handle)
    -> Texture*;

auto texture_create(TextureManager& texture_manager, const uint8_t* image_data,
                    int width, int height) -> TextureHandle;

auto texture_destroy(TextureManager& texture_manager, TextureHandle handle)
    -> void;

#endif  // TEXTURE_H
