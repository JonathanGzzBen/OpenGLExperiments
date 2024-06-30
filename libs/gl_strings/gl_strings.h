#ifndef GL_STRINGS_H
#define GL_STRINGS_H

#include <GL/glew.h>

namespace gl_strings {

auto source(GLenum source_enum) -> const char*;
auto severity(GLenum severity_enum) -> const char*;
auto type(GLenum type_enum) -> const char*;

}

#endif  // GL_STRINGS_H
