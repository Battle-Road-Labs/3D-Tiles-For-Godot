#ifndef MISSING_FUNCTIONS_H
#define MISSING_FUNCTIONS_H


#if defined(CESIUM_GD_EXT)
#include "godot_cpp/classes/file_access.hpp"
using namespace godot;
#elif defined(CESIUM_GD_MODULE)
//TODO:
#endif

Ref<FileAccess> open_file_access_with_err(const String &p_path, FileAccess::ModeFlags p_flags, Error* err);

#endif
