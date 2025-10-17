#ifndef MESH_H
#define MESH_H

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>

using Vertex = struct Vertex {
  glm::vec3 position;
  glm::vec2 uv;
};

using MeshData = struct MeshData {
  bool valid;
  const Vertex *vertices;
  size_t num_vertices;
  const unsigned int *indices;
  size_t num_indices;
};

using MeshHandle = int;

using MeshManager = struct MeshManager {
  bool valid;
  // Mesh *meshes;
  int meshes_count;
  int max_num_meshes;

  unsigned int *vbos;
  unsigned int *ebos;
  size_t *num_indices_s;
};

auto mesh_manager_create(int max_num_meshes) -> MeshManager;

auto mesh_manager_destroy_all(MeshManager *manager) -> void;

auto mesh_validate_handle(const MeshManager &manager, MeshHandle handle)
    -> bool;

// Causes internal fragmentation
auto mesh_destroy(const MeshManager *manager, MeshHandle handle) -> void;

auto mesh_create(MeshManager &manager, const MeshData &mesh_data)
    -> MeshHandle;

auto mesh_draw(const MeshManager &manager, MeshHandle handle) -> void;

#endif  // MESH_H
