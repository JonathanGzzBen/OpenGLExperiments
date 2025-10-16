#include "jtr/vertex_array_object.h"

#include <GL/glew.h>

#include <iostream>
#include <print>

auto vertex_array_object_manager_create(const int max_num_vaos)
    -> VertexArrayObjectManager {
  auto* vao_ids = new unsigned int[max_num_vaos];

  return VertexArrayObjectManager{.valid = true,
                                  .vao_count = 0,
                                  .max_num_vaos = max_num_vaos,
                                  .vao_ids = vao_ids};
}

auto vertex_array_object_manager_destroy_all(VertexArrayObjectManager* manager)
    -> void {
  if (manager == nullptr || !manager->valid) {
    std::println("VertexArrayObjectManager not valid");
    return;
  }
  for (VertexArrayObjectHandle handle = 0; handle < manager->max_num_vaos;
       ++handle) {
    vertex_array_object_destroy(*manager, handle);
  }
  delete manager->vao_ids;
  ;
  manager->vao_ids = nullptr;
  manager->vao_count = 0;
  manager->max_num_vaos = 0;
  manager->valid = false;
}

auto vertex_array_object_validate_handle(
    const VertexArrayObjectManager& manager,
    const VertexArrayObjectHandle handle) -> bool {
  if (!manager.valid) {
    std::println("VertexArrayObjectManager not valid");
    return false;
  }
  if (manager.vao_ids == nullptr || manager.vao_count == 0) {
    std::println("VertexArrayObjectManager not valid");
    return false;
  }
  if (manager.vao_count <= handle || manager.vao_ids[handle] < 0) {
    std::println(std::cerr, "VertexArrayObjectHandle not valid");
    return false;
  }
  return true;
}

auto vertex_array_object_destroy(const VertexArrayObjectManager& manager,
                                 const VertexArrayObjectHandle handle) -> void {
  if (!manager.valid) {
    std::println(std::cerr, "Mesh manager not valid");
    return;
  }
  if (!vertex_array_object_validate_handle(manager, handle)) {
    std::println(std::cerr, "Invalid handle");
    return;
  }
  glDeleteVertexArrays(1, &manager.vao_ids[handle]);
  manager.vao_ids[handle] = -1;
}

auto vertex_array_object_create(VertexArrayObjectManager* manager,
                                const VertexArrayAttributeEntry* attributes,
                                const size_t attributes_len)
    -> VertexArrayObjectHandle {
  unsigned int vao;
  glCreateVertexArrays(1, &vao);
  for (size_t i = 0; i < attributes_len; ++i) {
    const auto [index, size, type, normalized, relative_offset, binding_index] =
        attributes[i];
    glVertexArrayAttribFormat(vao, index, size, type, normalized,
                              relative_offset);
    glVertexArrayAttribBinding(vao, index, binding_index);
    glEnableVertexArrayAttrib(vao, index);
  }

  const auto handle = manager->vao_count++;
  manager->vao_ids[handle] = vao;
  return handle;
}

auto vertex_array_object_bind(const VertexArrayObjectManager& manager,
                              const VertexArrayObjectHandle handle) -> void {
  if (!manager.valid) {
    std::println(std::cerr, "Mesh manager not valid");
    return;
  }
  if (!vertex_array_object_validate_handle(manager, handle)) {
    std::println(std::cerr, "Invalid handle");
    return;
  }
  glBindVertexArray(manager.vao_ids[handle]);
}
