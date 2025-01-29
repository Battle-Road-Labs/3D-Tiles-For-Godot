#include "missing_functions.hpp"

#if defined(CESIUM_GD_EXT)
#include "godot_cpp/classes/file_access.hpp"
using namespace godot;


Ref<FileAccess> open_file_access_with_err(const String &p_path, FileAccess::ModeFlags p_flags, Error* err) {
  Ref<FileAccess> result = FileAccess::open(p_path, p_flags);
  *err = FileAccess::get_open_error();
  return result;
}

#elif defined(CESIUM_GD_MODULE)

Ref<FileAccess> open_file_access_with_err(const String &p_path, FileAccess::ModeFlags p_flags, Error* err) {
  return FileAccess::open(p_path, p_flags, err);
}
#endif
