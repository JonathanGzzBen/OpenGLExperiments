#ifndef ERROR_H
#define ERROR_H

#include <string>

namespace camera_control {
using Error = struct Error {
  std::string message;
};

inline auto with_context(Error err, const std::string& context) -> Error {
  err.message = context + ": " + err.message;
  return err;
}
} // namespace camera_control_textures

#endif //ERROR_H