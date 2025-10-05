#include "jtr/mesh.h"

#include <GL/glew.h>

#include <iostream>
#include <print>

auto mesh_manager_create(int max_num_meshes) -> MeshManager {
  if (max_num_meshes <= 0) {
    std::println(std::cerr, "Invalid max number of meshes");
    return {.valid = false};
  }
  const auto meshes_ptr = new Mesh[max_num_meshes];
  return MeshManager{.valid = true,
                     .meshes = meshes_ptr,
                     .meshes_count = 0,
                     .max_num_meshes = max_num_meshes};
}

auto mesh_manager_destroy_all(MeshManager &mesh_manager) -> void {
  if (!mesh_manager.valid) {
    std::println(std::cerr, "Mesh manager not valid");
    return;
  }
  for (size_t i = 0; i < mesh_manager.meshes_count; ++i) {
    glDeleteBuffers(1, &mesh_manager.meshes[i].vbo);
    glDeleteBuffers(1, &mesh_manager.meshes[i].ebo);
  }
  delete[] mesh_manager.meshes;
  mesh_manager.meshes = nullptr;
  mesh_manager.meshes_count = 0;
  mesh_manager.valid = false;
}

auto mesh_get(const MeshManager &mesh_manager, MeshHandle handle) -> Mesh * {
  if (!mesh_manager.valid) {
    std::println(std::cerr, "Mesh manager not valid");
    return nullptr;
  }
  if (handle < 0 || mesh_manager.meshes_count <= handle) {
    std::println(std::cerr, "Invalid handle");
    return nullptr;
  }
  if (!mesh_manager.meshes[handle].valid) {
    std::println(std::cerr, "Mesh for handle not valid");
    return nullptr;
  }
  return &mesh_manager.meshes[handle];
}

// Causes internal fragmentation
auto mesh_destroy(const MeshManager &mesh_manager, const MeshHandle handle)
    -> void {
  if (!mesh_manager.valid) {
    std::println(std::cerr, "Mesh manager not valid");
    return;
  }
  auto *const mesh = mesh_get(mesh_manager, handle);
  if (mesh == nullptr) {
    std::println(std::cerr, "Invalid handle");
    return;
  }
  glDeleteBuffers(1, &mesh->vbo);
  glDeleteBuffers(1, &mesh->ebo);
  mesh->valid = false;
}

auto mesh_create(MeshManager &mesh_manager, const MeshData &mesh_data)
    -> MeshHandle {
  if (!mesh_manager.valid) {
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
  if (mesh_manager.max_num_meshes <= mesh_manager.meshes_count) {
    std::println(std::cerr, "Maximum number of meshes ({}) exceeded",
                 mesh_manager.max_num_meshes);
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

  const size_t handle = mesh_manager.meshes_count++;
  mesh_manager.meshes[handle] = Mesh{
      .valid = true,
      .vbo = vbo,
      .ebo = ebo,
      .num_indices = mesh_data.num_indices,
  };
  return handle;
}
