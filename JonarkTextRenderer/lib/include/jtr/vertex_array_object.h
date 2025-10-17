#ifndef VERTEX_ARRAY_OBJECT_H
#define VERTEX_ARRAY_OBJECT_H
#include <GL/glew.h>

using VertexArrayObjectHandle = int;

using VertexArrayObjectManager = struct VertexArrayObjectManager {
  bool valid;
  int vao_count;
  int max_num_vaos;

  unsigned int *vao_ids;
};

using VertexArrayAttributeEntry = struct {
  int index;
  int size;
  GLenum type;
  GLboolean normalized;
  size_t relative_offset;
  int binding_index;
};

auto vertex_array_object_manager_create(int max_num_vaos)
    -> VertexArrayObjectManager;

auto vertex_array_object_manager_destroy_all(VertexArrayObjectManager *manager)
    -> void;

auto vertex_array_object_validate_handle(const VertexArrayObjectManager &manager,
                             VertexArrayObjectHandle handle)
    -> bool;

auto vertex_array_object_destroy(const VertexArrayObjectManager &manager,
                                 VertexArrayObjectHandle handle) -> void;

auto vertex_array_object_create(VertexArrayObjectManager *manager,
                                const VertexArrayAttributeEntry *attributes,
                                size_t attributes_len)
    -> VertexArrayObjectHandle;

auto vertex_array_object_bind(const VertexArrayObjectManager &manager,
                              VertexArrayObjectHandle handle) -> void;

#endif  // VERTEX_ARRAY_OBJECT_H
