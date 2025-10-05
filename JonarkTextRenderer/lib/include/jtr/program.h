#ifndef PROGRAM_H
#define PROGRAM_H

using ProgramHandle = int;

using Program = struct Program {
  bool valid;
  unsigned int program_id;
};

using ProgramManager = struct ProgramManager {
  bool valid;
  Program *programs;
  int programs_count;
  int max_num_programs;
};

auto program_manager_create(int max_num_programs) -> ProgramManager;

auto program_manager_destroy_all(ProgramManager &program_manager) -> void;

auto program_get(const ProgramManager &program_manager,
                 const ProgramHandle handle) -> Program *;

// Causes internal fragmentation
auto program_destroy(const ProgramManager &program_manager,
                     const ProgramHandle handle) -> void;

auto program_create(ProgramManager &program_manager,
                    const char *vertex_shader_source,
                    const char *fragment_shader_source) -> ProgramHandle;

#endif  // PROGRAM_H
