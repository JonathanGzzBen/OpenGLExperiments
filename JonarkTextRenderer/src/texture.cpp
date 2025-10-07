#include "jtr/texture.h"

auto texture_manager_create(const int max_num_textures) -> TextureManager {
  if (max_num_textures <= 0) {
    std::println(stderr, "Invalid max_num_textures");
    return TextureManager{.valid = false};
  }
  return TextureManager{
      .valid = true,
      .textures = new Texture[max_num_textures],
      .num_textures = 0,
      .max_num_textures = max_num_textures,
  };
}

auto texture_manager_destroy_all(TextureManager* manager) -> void {
  for (int i = 0; i < manager->num_textures; ++i) {
    texture_destroy(*manager, static_cast<TextureHandle>(i));
  }
  delete[] manager->textures;
  manager->num_textures = 0;
  manager->valid = false;
}

auto texture_get(const TextureManager& manager, const TextureHandle handle)
    -> Texture* {
  if (!manager.valid) {
    std::println(stderr, "Texture Manager not valid");
    return nullptr;
  }
  if (handle < 0 || manager.max_num_textures <= handle) {
    std::println(stderr, "Invalid handle");
    return nullptr;
  }
  if (!manager.textures[handle].valid) {
    std::println(std::cerr, "Texture for handle not valid");
    return nullptr;
  }
  return &manager.textures[handle];
}

auto texture_create(TextureManager& texture_manager, const uint8_t* image_data,
                    const int width, const int height) -> TextureHandle {
  unsigned int texture_id;
  glCreateTextures(GL_TEXTURE_2D, 1, &texture_id);
  if (texture_id == 0) {
    std::println(stderr, "Could not create texture");
    return -1;
  }

  glTextureParameteri(texture_id, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTextureParameteri(texture_id, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTextureParameteri(texture_id, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTextureParameteri(texture_id, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

  glTextureStorage2D(texture_id, 1, GL_R8, width, height);
  glTextureSubImage2D(texture_id, 0, 0, 0, width, height, GL_RED,
                      GL_UNSIGNED_BYTE, image_data);

  glGenerateTextureMipmap(texture_id);

  const auto handle = texture_manager.num_textures++;
  texture_manager.textures[handle] = Texture{
      .valid = true,
      .texture_id = texture_id,
  };
  return handle;
}

auto texture_destroy(TextureManager& texture_manager, TextureHandle handle)
    -> void {
  if (!texture_manager.valid) {
    std::println(stderr, "Texture Manager not valid");
    return;
  }
  if (handle < 0 || texture_manager.max_num_textures <= handle) {
    std::println(stderr, "Invalid handle");
    return;
  }
  if (!texture_manager.textures[handle].valid) {
    std::println(std::cerr, "Texture for handle not valid");
    return;
  }
  glDeleteTextures(1, &texture_manager.textures[handle].texture_id);
  texture_manager.textures[handle] = Texture{.valid = false};
}
