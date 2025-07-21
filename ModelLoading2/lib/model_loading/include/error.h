#ifndef ERROR_H
#define ERROR_H

#include <string>

namespace model_loading {
using Error = struct Error {
  std::string message;
};

inline auto with_context(Error err, const std::string& context) -> Error {
  err.message = context + ": " + err.message;
  return err;
}
} // namespace model_loading

#endif //ERROR_H