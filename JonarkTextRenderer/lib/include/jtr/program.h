#ifndef PROGRAM_H
#define PROGRAM_H

using ProgramHandle = int;

using Program = struct Program {
  bool valid;
  unsigned int program_id;
};

using ProgramManager = struct ProgramManager {
  bool valid;
  // Program *programs;
  int programs_count;
  int max_num_programs;
  unsigned int *program_ids;
};

auto program_manager_create(int max_num_programs) -> ProgramManager;

auto program_manager_destroy_all(ProgramManager *program_manager) -> void;

auto program_validate_handle(const ProgramManager &program_manager, ProgramHandle handle)
    -> bool;

// Causes internal fragmentation
auto program_destroy(const ProgramManager &manager,
                     ProgramHandle handle) -> void;

auto program_create(ProgramManager &program_manager,
                    const char *vertex_shader_source,
                    const char *fragment_shader_source) -> ProgramHandle;

auto program_use(const ProgramManager &manager, ProgramHandle handle)
    -> void;

template <typename T>
auto program_set_uniform(const ProgramManager &program_manager,
                         ProgramHandle handle, const char *uniform_name,
                         const T value) -> void = delete;

template <>
auto program_set_uniform(const ProgramManager &program_manager,
                         ProgramHandle handle, const char *uniform_name,
                         int value) -> void;

#endif  // PROGRAM_H
