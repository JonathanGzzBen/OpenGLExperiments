#ifndef ERROR_H
#define ERROR_H

#include <string>

namespace lighting {
using Error = struct Error {
  std::string message;
};

inline auto with_context(Error err, const std::string& context) -> Error {
  err.message = context + ": " + err.message;
  return err;
}
} // namespace lighting

#endif //ERROR_H