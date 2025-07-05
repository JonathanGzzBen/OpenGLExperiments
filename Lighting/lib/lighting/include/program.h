#ifndef PROGRAM_H
#define PROGRAM_H

#include <GL/glew.h>

#include <expected>
#include <format>
#include <fstream>
#include <string>
#include <sstream>
#include  "error.h"

#include  "gl/gl.h"

namespace lighting {
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

  auto Use() const -> void;

  static auto Create(const std::string& vertex_shader_filename,
                     const std::string& fragment_shader_filename) ->
    std::expected<Program, Error>;
};
} // namespace lighting


#endif //PROGRAM_H