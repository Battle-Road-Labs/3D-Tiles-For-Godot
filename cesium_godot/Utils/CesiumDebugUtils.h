#ifndef CESIUM_DEBUG_UTILS_H
#define CESIUM_DEBUG_UTILS_H

#if defined(CESIUM_GD_EXT)
#include <godot_cpp/classes/node3d.hpp>
using namespace godot;
#elif defined(CESIUM_GD_MODULE)
#include "scene/3d/node_3d.h"
#endif

class CesiumDebugUtils : public Object {
	GDCLASS(CesiumDebugUtils, Object)

public:
	static void draw_line(const Vector3& from, const Vector3& to, const Color& color);

	static void draw_wire_sphere(const Vector3& origin, real_t radius, const Color& color);

	static void draw_thick_line(const Vector3& from, const Vector3& to, real_t width, const Color& color);

protected:
	static void _bind_methods();

};

#endif // !DEBUG_UTILS_H
