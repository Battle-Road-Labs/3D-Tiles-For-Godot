#include "CesiumDebugUtils.h"
#if defined(CESIUM_GD_EXT)
#include "godot_cpp/classes/rendering_server.hpp"
#include <godot_cpp/templates/vector.hpp>
#include <godot_cpp/classes/array_mesh.hpp>
#endif

void CesiumDebugUtils::draw_line(const Vector3& from, const Vector3& to, const Color& color)
{

}

void CesiumDebugUtils::draw_wire_sphere(const Vector3& origin, real_t radius, const Color& color)
{

}

void CesiumDebugUtils::draw_thick_line(const Vector3& from, const Vector3& to, real_t width, const Color& color)
{

}

void CesiumDebugUtils::_bind_methods()
{
	ClassDB::bind_static_method("CesiumDebugUtils", D_METHOD("draw_line", "from", "to", "color"), &CesiumDebugUtils::draw_line);
}
