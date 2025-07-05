#ifndef MESH_H
#define MESH_H

#include  <GL/glew.h>

#include  <vector>
#include  <expected>

#include "error.h"

namespace lighting {
using Vertex = struct Vertex {
  float x, y, z, u, v, nx, ny, nz;
};


class Mesh {
private:
  unsigned int vbo = 0;
  unsigned int ebo = 0;
  size_t indices_count = 0;

  Mesh(const unsigned int vbo, const unsigned int ebo,
       const size_t indices_count);

public:
  // Delete copy constructors
  Mesh(const Mesh&) = delete;
  auto operator=(const Mesh&) -> Mesh& = delete;

  // Take ownership
  Mesh(Mesh&& other) noexcept;

  ~Mesh();

  static auto Create(const std::vector<Vertex>& vertices,
                     const std::vector<unsigned int>& indices) -> std::expected<
    Mesh, Error>;

  auto Draw(const unsigned int vao,
            const unsigned int binding_index) const -> void;
};
} // namespace lighting

#endif //MESH_H