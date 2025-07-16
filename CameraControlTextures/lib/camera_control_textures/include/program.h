#ifndef PROGRAM_H
#define PROGRAM_H

#include <GL/glew.h>

#include <expected>
#include <format>
#include <string>
#include <glm/ext/matrix_float4x4.hpp>

#include  "error.h"

#include  "gl/gl.h"

namespace camera_control {
class Program {
private:
  unsigned int program_id_ = 0;

  explicit Program(unsigned int program_id);

  static auto read_file(
      const std::string& filename) -> std::expected<std::string, Error>;

  static auto compile_shader(const GLenum type,
                             const std::string& filename) -> std::expected<
    GLuint, Error>;

public:
  // Delete copy constructors
  Program(const Program&) = delete;
  auto operator=(const Program&) -> Program& = delete;

  // Take ownership of the program_id
  Program(Program&& other) noexcept;

  ~Program();

  static auto Create(const std::string& vertex_shader_filename,
                     const std::string& fragment_shader_filename) ->
    std::expected<Program, Error>;

  auto Use() const -> void;

  [[nodiscard]] auto SetUniformMatrix(const std::string& uniform_name,
                                      const glm::mat4& matrix) const ->
    std::expected<
      void, Error>;

  [[nodiscard]] auto SetUniformV3(const std::string& uniform_name,
                           const glm::vec3& vec) const ->
    std::expected<
      void, Error>;
};
} // namespace camera_control_textures


#endif //PROGRAM_H