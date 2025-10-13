#include "jtr/vertex_array_object.h"

#include <GL/glew.h>

#include <iostream>
#include <print>

auto vertex_array_object_manager_create(const int max_num_vaos)
    -> VertexArrayObjectManager {
  auto* vaos = new VertexArrayObject[max_num_vaos];

  return VertexArrayObjectManager{
      .valid = true,
      .vaos = vaos,
      .vao_count = 0,
      .max_num_vaos = max_num_vaos,
  };
}

auto vertex_array_object_manager_destroy_all(VertexArrayObjectManager* manager)
    -> void {
  if (manager == nullptr || !manager->valid) {
    std::println("VertexArrayObjectManager not valid");
    return;
  }
  for (VertexArrayObjectHandle handle = 0; handle < manager->max_num_vaos; ++handle) {
    vertex_array_object_destroy(*manager, handle);
  }
  delete manager->vaos;
  manager->vaos = nullptr;
  manager->vao_count = 0;
  manager->max_num_vaos = 0;
  manager->valid = false;
}

auto vertex_array_object_get(const VertexArrayObjectManager& manager,
                             const VertexArrayObjectHandle handle)
    -> VertexArrayObject* {
  if (!manager.valid) {
    std::println("VertexArrayObjectManager not valid");
    return nullptr;
  }
  if (manager.vaos == nullptr || manager.vao_count == 0) {
    std::println("VertexArrayObjectManager not valid");
    return nullptr;
  }
  if (manager.vao_count <= handle || !manager.vaos[handle].valid) {
    std::println(std::cerr, "VertexArrayObjectHandle not valid");
    return nullptr;
  }
  return &manager.vaos[handle];
}

auto vertex_array_object_destroy(const VertexArrayObjectManager& manager,
                                 VertexArrayObjectHandle handle) -> void {
  if (!manager.valid) {
    std::println(std::cerr, "Mesh manager not valid");
    return;
  }
  auto* const vao = vertex_array_object_get(manager, handle);
  if (vao == nullptr || !vao->valid) {
    std::println(std::cerr, "Invalid handle");
    return;
  }
  glDeleteVertexArrays(1, &vao->vao_id);
  vao->valid = false;
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
  manager->vaos[handle] = VertexArrayObject{.valid = true, .vao_id = vao};
  return handle;
}
