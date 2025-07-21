#ifndef MESH_H
#define MESH_H

#include <GL/glew.h>

#include <expected>
#include <vector>

#include "error.h"
#include "program.h"

namespace model_loading {
using Vertex = struct Vertex {
  glm::vec3 position;
  glm::vec3 normal;
  glm::vec2 uv;
};

using Texture = struct Texture {
  unsigned int id;
  std::string type;
};

class Mesh {
 private:
  unsigned int vao_;
  unsigned int vbo_;
  unsigned int ebo_;

  void setupMesh();

 public:
  std::vector<Vertex> vertices;
  std::vector<unsigned int> indices;
  std::vector<Texture> textures;

  Mesh(std::vector<Vertex> vertices, std::vector<unsigned int> indices,
       std::vector<Texture> textures);

  auto Draw(const Program& program, const unsigned int vao) -> void;
};

}  // namespace model_loading

#endif  // MESH_H