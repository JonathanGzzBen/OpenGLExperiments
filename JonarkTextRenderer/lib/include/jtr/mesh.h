#ifndef MESH_H
#define MESH_H

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>

using Vertex = struct Vertex {
  glm::vec3 position;
  // glm::vec3 normal;
  glm::vec2 uv;
};

using MeshData = struct MeshData {
  bool valid;
  Vertex *vertices;
  size_t num_vertices;
  unsigned int *indices;
  size_t num_indices;
  unsigned int texture_id;
};

using Mesh = struct Mesh {
  bool valid;
  unsigned int vbo;
  unsigned int ebo;
  size_t num_indices;
};

using MeshHandle = int;

using MeshManager = struct MeshManager {
  bool valid;
  Mesh *meshes;
  int meshes_count;
  int max_num_meshes;
};

auto mesh_manager_create(int max_num_meshes) -> MeshManager;

auto mesh_manager_destroy_all(MeshManager &mesh_manager) -> void;

auto mesh_get(const MeshManager &mesh_manager, MeshHandle handle) -> Mesh *;

// Causes internal fragmentation
auto mesh_destroy(const MeshManager &mesh_manager, const MeshHandle handle)
    -> void;

auto mesh_create(MeshManager &mesh_manager, const MeshData &mesh_data)
    -> MeshHandle;

#endif  // MESH_H
