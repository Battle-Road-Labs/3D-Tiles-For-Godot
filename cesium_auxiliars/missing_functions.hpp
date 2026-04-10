#ifndef MISSING_FUNCTIONS_H
#define MISSING_FUNCTIONS_H


#if defined(CESIUM_GD_EXT)
#include "godot_cpp/classes/file_access.hpp"
using namespace godot;
#elif defined(CESIUM_GD_MODULE)
#include "core/io/file_access.h"
#include "core/config/engine.h"
#endif

Ref<FileAccess> open_file_access_with_err(const String &p_path, FileAccess::ModeFlags p_flags, Error* err);

bool is_editor_mode();

#define PRINT_VEC3(v) printf("(%.2f, %.2f, %.2f)\n", v.x, v.y, v.z)

#endif
