#include "gl_strings.h"

namespace gl_strings {

auto source(GLenum source_enum) -> const char* {
  switch (source_enum) {
    case GL_DEBUG_SOURCE_API:
      return static_cast<const char*>("API");
    case GL_DEBUG_SOURCE_WINDOW_SYSTEM:
      return static_cast<const char*>("WINDOW SYSTEM");
    case GL_DEBUG_SOURCE_SHADER_COMPILER:
      return static_cast<const char*>("SHADER COMPILER");
    case GL_DEBUG_SOURCE_THIRD_PARTY:
      return static_cast<const char*>("THIRD PARTY");
    case GL_DEBUG_SOURCE_APPLICATION:
      return static_cast<const char*>("APPLICATION");
    case GL_DEBUG_SOURCE_OTHER:
      return static_cast<const char*>("OTHER");
    default:
      return static_cast<const char*>("UNKNOWN");
  }
}

auto severity(GLenum severity_enum) -> const char* {
  switch (severity_enum) {
    case GL_DEBUG_SEVERITY_HIGH:
      return static_cast<const char*>("HIGH");
    case GL_DEBUG_SEVERITY_MEDIUM:
      return static_cast<const char*>("MEDIUM");
    case GL_DEBUG_SEVERITY_LOW:
      return static_cast<const char*>("LOW");
    case GL_DEBUG_SEVERITY_NOTIFICATION:
      return static_cast<const char*>("NOTIFICATION");
    default:
      return static_cast<const char*>("UNKNOWN");
  }
}

auto type(GLenum type_enum) -> const char* {
  switch (type_enum) {
    case GL_DEBUG_TYPE_ERROR:
      return static_cast<const char*>("ERROR");
    case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR:
      return static_cast<const char*>("DEPRECATED BEHAVIOR");
    case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:
      return static_cast<const char*>("UNDEFINED BEHAVIOR");
    case GL_DEBUG_TYPE_PORTABILITY:
      return static_cast<const char*>("PORTABILITY");
    case GL_DEBUG_TYPE_PERFORMANCE:
      return static_cast<const char*>("PERFORMANCE");
    case GL_DEBUG_TYPE_MARKER:
      return static_cast<const char*>("MARKER");
    case GL_DEBUG_TYPE_OTHER:
      return static_cast<const char*>("OTHER");
    default:
      return static_cast<const char*>("UNKNOWN");
  }
}

}  // namespace gl_strings
