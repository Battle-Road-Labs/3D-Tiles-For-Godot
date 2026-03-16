#ifndef CESIUM_TEST_CLASS_H
#define CESIUM_TEST_CLASS_H

//Make Cesium not check for thread safety
#define NDEBUG


#if defined(CESIUM_GD_EXT)
#include <godot_cpp/classes/node3d.hpp>
using namespace godot;
#elif defined(CESIUM_GD_MODULE)
#include "scene/3d/node_3d.h"
#endif

class CesiumTestClass : public Node3D {
	GDCLASS(CesiumTestClass, Node3D)
protected:
	static void _bind_methods();
};

#endif // CESIUM_TEST_CLASS_H