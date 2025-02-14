#ifndef GDHTML_REGISTER_TYPES_H
#define GDHTML_REGISTER_TYPES_H

#include <godot_cpp/core/class_db.hpp>

using namespace godot;

void initialize_html_module(ModuleInitializationLevel p_level);
void uninitialize_html_module(ModuleInitializationLevel p_level);

#endif // GDHTML_REGISTER_TYPES_H