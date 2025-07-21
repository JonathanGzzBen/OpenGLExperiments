#include  "mesh.h"

model_loading::Mesh::Mesh(const unsigned int vbo, const unsigned int ebo,
                     const size_t indices_count): vbo(vbo),
                                                  ebo(ebo),
                                                  indices_count(indices_count) {
}

model_loading::Mesh::Mesh(Mesh&& other) noexcept {
  std::swap(vbo, other.vbo);
  std::swap(ebo, other.ebo);
  std::swap(indices_count, other.indices_count);
}

model_loading::Mesh::~Mesh() {
  if (glIsBuffer(vbo)) {
    glDeleteBuffers(1, &vbo);
  }
  if (glIsBuffer(ebo)) {
    glDeleteBuffers(1, &ebo);
  }
}

auto model_loading::Mesh::Create(const std::vector<Vertex>& vertices,
                            const std::vector<unsigned int>& indices) ->
  std::expected<Mesh, Error> {
  unsigned int vbo;
  glCreateBuffers(1, &vbo);
  glNamedBufferStorage(
      vbo, static_cast<GLsizei>(vertices.size() * sizeof(Vertex)),
      vertices.data(),
      0);

  unsigned int ebo;
  glCreateBuffers(1, &ebo);
  glNamedBufferStorage(
      ebo, static_cast<GLsizei>(indices.size() * sizeof(unsigned int)),
      indices.data(), 0);

  return {Mesh(vbo, ebo, indices.size())};
}

auto model_loading::Mesh::Draw(const Program& program, const unsigned int vao,
                          const unsigned int binding_index) const -> void {
  program.Use();
  glBindVertexArray(vao);
  glVertexArrayVertexBuffer(vao, binding_index, vbo, 0, sizeof(Vertex));
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
  glDrawElements(GL_TRIANGLES, indices_count, GL_UNSIGNED_INT, 0);
}