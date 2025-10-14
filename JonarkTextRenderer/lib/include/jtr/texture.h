#ifndef TEXTURE_H
#define TEXTURE_H
#include <GL/glew.h>

#include <iostream>
#include <print>

using TextureHandle = int;

using TextureManager = struct TextureManager {
  bool valid;
  int num_textures;
  int max_num_textures;

  unsigned int* texture_ids;
};

auto texture_manager_create(int max_num_textures) -> TextureManager;

auto texture_manager_destroy_all(TextureManager* manager) -> void;

auto texture_validate_handle(const TextureManager& manager,
                             TextureHandle handle) -> bool;

auto texture_create(TextureManager& texture_manager, const uint8_t* image_data,
                    int width, int height) -> TextureHandle;

auto texture_destroy(const TextureManager& texture_manager,
                     TextureHandle handle) -> void;

auto texture_bind(const TextureManager& manager, TextureHandle handle,
                  int texture_unit) -> void;

#endif  // TEXTURE_H
