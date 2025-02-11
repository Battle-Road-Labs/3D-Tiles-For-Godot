#ifndef ERROR_NAMES_H
#define ERROR_NAMES_H

#if defined (CESIUM_GD_EXT)
#include <godot_cpp/classes/global_constants.hpp>
#include "magic_enum.hpp"

#define REFLECT_ERR_NAME(x) magic_enum::enum_name(x).data()

#elif defined(CESIUM_GD_MODULE)

//TODO: See if this will work
#define REFLECT_REFLECT_ERR_NAME(x) error_names[x]

#endif
#endif
