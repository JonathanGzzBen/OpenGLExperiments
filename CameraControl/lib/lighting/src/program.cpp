#include "program.h"

#include <sstream>
#include <fstream>

namespace lighting {
Program::Program(unsigned int program_id) : program_id_(program_id) {
}

auto Program::read_file(
    const std::string& filename) -> std::expected<std::string, Error> {
  std::ifstream input_stream(filename);
  std::stringstream file_contents_stream;

  if (!input_stream.is_open() || input_stream.fail()) {
    return std::unexpected(
        Error{.message = std::format("Could not open file '{}'", filename)});
  }
  file_contents_stream << input_stream.rdbuf();
  return {file_contents_stream.str()};
}

auto Program::compile_shader(const GLenum type,
                             const std::string& filename) -> std::expected<
  GLuint, Error> {
  const auto source = read_file(filename);
  if (!source) {
    return std::unexpected(with_context(source.error(),
                                        std::format(
                                            "Error getting source for vertex type {}",
                                            type)));
  }

  const auto shader = glCreateShader(type);
  const auto source_ptr = source->c_str();
  glShaderSource(shader, 1, &source_ptr,
                 nullptr);

  glCompileShader(shader);
  auto shader_compile_status = GL_FALSE;
  glGetShaderiv(shader, GL_COMPILE_STATUS, &shader_compile_status);
  if (shader_compile_status != GL_TRUE) {
    char message[1024] = {0};
    GLsizei message_len = 0;
    glGetShaderInfoLog(shader, sizeof(message), &message_len, message);
    return std::unexpected(
        Error{.message =
            std::format("Could not compile shader type {}: {}", type,
                        message)});
  }
  return shader;
}

Program::Program(Program&& other) noexcept {
  std::swap(program_id_, other.program_id_);
}

Program::~Program() {
  glDeleteProgram(program_id_);
}

auto Program::Create(const std::string& vertex_shader_filename,
                     const std::string& fragment_shader_filename) ->
  std::expected<Program, Error> {
  const auto vertex_shader = compile_shader(
      GL_VERTEX_SHADER, vertex_shader_filename);
  if (!vertex_shader) {
    return std::unexpected(with_context(vertex_shader.error(),
                                        "Could not compile vertex shader"));
  }
  const auto fragment_shader = compile_shader(
      GL_FRAGMENT_SHADER, fragment_shader_filename);
  if (!fragment_shader) {
    return std::unexpected(with_context(fragment_shader.error(),
                                        "Could not compile fragment shader"));
  }

  const auto program_id = glCreateProgram();
  glAttachShader(program_id, *vertex_shader);
  glAttachShader(program_id, *fragment_shader);
  glLinkProgram(program_id);
  auto program_link_status = GL_FALSE;
  glGetProgramiv(program_id, GL_LINK_STATUS, &program_link_status);
  if (program_link_status != GL_TRUE) {
    return std::unexpected(Error{
        .message = std::format("Could not link program status: {}",
                               program_link_status)});
  }

  return Program{program_id};
}

auto Program::Use() const -> void {
  glUseProgram(program_id_);
}

auto Program::SetUniformMatrix(const std::string& uniform_name,
                               const glm::mat4& matrix) const -> std::expected<
  void, Error> {
  glUseProgram(program_id_);
  const auto location = glGetUniformLocation(program_id_, uniform_name.c_str());
  if (location == -1) {
    return std::unexpected(Error{
        .message = std::format("Could not get uniform location of '{}'",
                               uniform_name)});
  }

  glUniformMatrix4fv(location, 1, GL_FALSE, &matrix[0][0]);
  glUseProgram(0);
  return {};
}

auto Program::SetUniformV3(const std::string& uniform_name,
                    const glm::vec3& vec) const -> std::expected<void, Error> {
  glUseProgram(program_id_);
  const auto location = glGetUniformLocation(program_id_, uniform_name.c_str());
  if (location == -1) {
    return std::unexpected(Error{
        .message = std::format("Could not get uniform location of '{}'",
                               uniform_name)});
  }

  glUniform3f(location, vec.x, vec.y, vec.z);
  glUseProgram(0);
  return {};
}
} // namespace lighting