#ifndef CESIUM_GODOT_REGISTER_TYPES_H
#define CESIUM_GODOT_REGISTER_TYPES_H

#if defined(CESIUM_GD_EXT)
#include <godot_cpp/core/class_db.hpp>

void initialize_cesium_godot_module(godot::ModuleInitializationLevel p_level);
void uninitialize_cesium_godot_module(godot::ModuleInitializationLevel p_level);

#elif defined(CESIUM_GD_MODULE)
#include "modules/register_module_types.h"

void initialize_cesium_godot_module(ModuleInitializationLevel p_level);
void uninitialize_cesium_godot_module(ModuleInitializationLevel p_level);

#endif

#endif
