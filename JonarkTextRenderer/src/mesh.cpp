#include "jtr/mesh.h"

#include <GL/glew.h>

#include <iostream>
#include <print>

auto mesh_manager_create(int max_num_meshes) -> MeshManager {
  if (max_num_meshes <= 0) {
    std::println(std::cerr, "Invalid max number of meshes");
    return {.valid = false};
  }
  return MeshManager{
      .valid = true,
      // .meshes = meshes_ptr,
      .meshes_count = 0,
      .max_num_meshes = max_num_meshes,
      .vbos = new unsigned int[max_num_meshes],
      .ebos = new unsigned int[max_num_meshes],
      .num_indices_s = new size_t[max_num_meshes],
  };
}

auto mesh_manager_destroy_all(MeshManager *manager) -> void {
  if (!manager->valid) {
    std::println(std::cerr, "Mesh manager not valid");
    return;
  }
  for (int i = 0; i < manager->meshes_count; ++i) {
    glDeleteBuffers(1, &manager->vbos[i]);
    glDeleteBuffers(1, &manager->ebos[i]);
  }
  delete[] manager->vbos;
  delete[] manager->ebos;
  delete[] manager->num_indices_s;
  manager->vbos = nullptr;
  manager->ebos = nullptr;
  manager->num_indices_s = nullptr;
  manager->meshes_count = 0;
  manager->valid = false;
}

auto mesh_validate_handle(const MeshManager &manager, const MeshHandle handle)
    -> bool {
  if (!manager.valid) {
    std::println(std::cerr, "Mesh manager not valid");
    return false;
  }
  if (handle < 0 || manager.meshes_count <= handle) {
    std::println(std::cerr, "Invalid handle");
    return false;
  }
  if (manager.vbos[handle] < 0) {
    std::println(std::cerr, "Mesh for handle not valid");
    return false;
  }
  return true;
}

// Causes internal fragmentation
auto mesh_destroy(const MeshManager *manager, const MeshHandle handle) -> void {
  if (!manager->valid) {
    std::println(std::cerr, "Mesh manager not valid");
    return;
  }
  if (!mesh_validate_handle(*manager, handle)) {
    std::println(std::cerr, "Invalid handle");
    return;
  }
  glDeleteBuffers(1, &manager->vbos[handle]);
  glDeleteBuffers(1, &manager->ebos[handle]);
  manager->vbos[handle] = -1;
  manager->ebos[handle] = -1;
  manager->num_indices_s[handle] = 0;
}

auto mesh_create(MeshManager &manager, const MeshData &mesh_data)
    -> MeshHandle {
  if (!manager.valid) {
    std::println(std::cerr, "Mesh manager not valid");
    return -1;
  }
  if (!mesh_data.valid) {
    std::println(std::cerr, "Mesh data not valid");
    return -1;
  }
  if (mesh_data.num_vertices == 0 || mesh_data.vertices == nullptr) {
    std::println(std::cerr, "No vertices specified to create mesh");
    return -1;
  }
  if (mesh_data.num_indices == 0 || mesh_data.indices == nullptr) {
    std::println(std::cerr, "No indices specified to create mesh");
    return -1;
  }
  if (manager.max_num_meshes <= manager.meshes_count) {
    std::println(std::cerr, "Maximum number of meshes ({}) exceeded",
                 manager.max_num_meshes);
    return -1;
  }

  unsigned int vbo;
  glCreateBuffers(1, &vbo);
  unsigned int ebo;
  glCreateBuffers(1, &ebo);

  glNamedBufferStorage(vbo, sizeof(Vertex) * mesh_data.num_vertices,
                       mesh_data.vertices, GL_DYNAMIC_STORAGE_BIT);

  glNamedBufferStorage(ebo, sizeof(unsigned int) * mesh_data.num_indices,
                       mesh_data.indices, GL_DYNAMIC_STORAGE_BIT);

  const auto handle = manager.meshes_count++;
  manager.vbos[handle] = vbo;
  manager.ebos[handle] = ebo;
  manager.num_indices_s[handle] = mesh_data.num_indices;
  return handle;
}

auto mesh_draw(const MeshManager &manager, const MeshHandle handle) -> void {
  if (!mesh_validate_handle(manager, handle)) {
    std::println(std::cerr, "Invalid handle");
    return;
  }
  // Hard coded binding index, should come from a VAO manager/object
  glBindVertexBuffer(0, manager.vbos[handle], 0, sizeof(Vertex));
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, manager.ebos[handle]);

  glDrawElements(GL_TRIANGLES, static_cast<int>(manager.num_indices_s[handle]),
                 GL_UNSIGNED_INT, nullptr);
  glBindVertexBuffer(0, 0, 0, sizeof(Vertex));
}
