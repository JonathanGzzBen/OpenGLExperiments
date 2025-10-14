#include "jtr/program.h"

#include <gl/glew.h>

#include <print>

auto program_manager_create(const int max_num_programs) -> ProgramManager {
  if (max_num_programs <= 0) {
    std::println(stderr, "Invalid max number of programs");
    return ProgramManager{.valid = false};
  }
  auto *programs_ids_ptr = new unsigned int[max_num_programs];
  return ProgramManager{.valid = true,
                        .programs_count = 0,
                        .max_num_programs = max_num_programs,
                        .program_ids = programs_ids_ptr

  };
}

auto program_manager_destroy_all(ProgramManager *program_manager) -> void {
  if (!program_manager->valid) {
    std::println(stderr, "Invalid program manager");
    return;
  }
  for (ProgramHandle i = 0; i < program_manager->programs_count; ++i) {
    program_destroy(*program_manager, i);
  }
  delete[] program_manager->program_ids;
  program_manager->program_ids = nullptr;
  program_manager->programs_count = 0;
  program_manager->max_num_programs = 0;
  program_manager->valid = false;
}

auto program_validate_handle(const ProgramManager &program_manager,
                             const ProgramHandle handle) -> bool {
  if (!program_manager.valid) {
    std::println(stderr, "Invalid program manager");
    return false;
  }
  if (handle < 0 || program_manager.programs_count <= handle) {
    std::println(stderr, "Invalid handle");
    return false;
  }
  return true;
}

// Causes internal fragmentation
auto program_destroy(const ProgramManager &manager, const ProgramHandle handle)
    -> void {
  if (!program_validate_handle(manager, handle)) {
    std::println(stderr, "Invalid program");
    return;
  }
  glDeleteProgram(manager.program_ids[handle]);
  manager.program_ids[handle] = 0;
}

auto compile_shader(const GLenum shader_type, const char *source)
    -> unsigned int {
  const auto shader_id = glCreateShader(shader_type);
  glShaderSource(shader_id, 1, &source, nullptr);
  glCompileShader(shader_id);

  int compiled;
  glGetShaderiv(shader_id, GL_COMPILE_STATUS, &compiled);
  if (compiled != GL_TRUE) {
    GLsizei log_length = 0;
    static constexpr size_t compile_err_message_len = 1024;
    GLchar message[compile_err_message_len];
    glGetShaderInfoLog(shader_id, compile_err_message_len, &log_length,
                       message);
    std::println(stderr, "Could not compile shader {}: {}", shader_id, message);
    return 0;
  }
  return shader_id;
}

auto compile_and_link_program(const unsigned int vertex_shader,
                              const unsigned int fragment_shader)
    -> unsigned int {
  const auto program_id = glCreateProgram();
  glAttachShader(program_id, vertex_shader);
  glAttachShader(program_id, fragment_shader);
  glLinkProgram(program_id);
  int linked;
  glGetProgramiv(program_id, GL_LINK_STATUS, &linked);
  if (linked != GL_TRUE) {
    GLsizei log_length = 0;
    static constexpr size_t link_err_message_len = 1024;
    GLchar message[link_err_message_len];
    glGetProgramInfoLog(program_id, link_err_message_len, &log_length, message);
    std::println(stderr, "Could not link program {}: {}", program_id,
                 log_length);
    return 0;
  }
  return program_id;
}

auto program_create(ProgramManager &program_manager,
                    const char *vertex_shader_source,
                    const char *fragment_shader_source) -> ProgramHandle {
  if (!program_manager.valid) {
    std::println(stderr, "Invalid program manager");
    return -1;
  }

  const auto vertex_shader =
      compile_shader(GL_VERTEX_SHADER, vertex_shader_source);
  if (vertex_shader == 0) {
    std::println(stderr, "Could not compile vertex shader");
    return -1;
  }
  const auto fragment_shader =
      compile_shader(GL_FRAGMENT_SHADER, fragment_shader_source);
  if (fragment_shader == 0) {
    std::println(stderr, "Could not compile fragment shader");
    return -1;
  }

  const auto program_id =
      compile_and_link_program(vertex_shader, fragment_shader);
  if (program_id == 0) {
    std::println(stderr, "Could not link program");
    return -1;
  }

  const int handle = program_manager.programs_count++;
  program_manager.program_ids[handle] = program_id;
  return handle;
}

auto program_use(const ProgramManager &manager, const ProgramHandle handle)
    -> void {
  if (!program_validate_handle(manager, handle)) {
    std::println(stderr, "Invalid program");
    return;
  }
  glUseProgram(manager.program_ids[handle]);
}

template <>
auto program_set_uniform<int>(const ProgramManager &program_manager,
                              const ProgramHandle handle,
                              const char *uniform_name, const int value)
    -> void {
  if (!program_validate_handle(program_manager, handle)) {
    std::println(stderr, "Invalid program");
    return;
  }
  glUniform1i(
      glGetUniformLocation(program_manager.program_ids[handle], uniform_name),
      value);
}
