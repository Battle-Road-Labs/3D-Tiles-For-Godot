#include "CesiumGlobe.h"

#if defined(CESIUM_GD_EXT)
#include <godot_cpp/classes/viewport.hpp>
#include <godot_cpp/classes/camera3d.hpp>
#elif defined(CESIUM_GD_MODULE)
#include "scene/main/viewport.h"
#include "scene/3d/camera_3d.h"
#endif


#include "CesiumGeospatial/Ellipsoid.h"
#include "../Utils/CesiumMathUtils.h"

Transform3D CesiumGlobe::get_tx_engine_to_ecef() const
{
	return this->get_global_transform().inverse();
}

Transform3D CesiumGlobe::get_tx_ecef_to_engine() const
{
	return this->get_global_transform();
}

Transform3D CesiumGlobe::get_initial_tx_ecef_to_engine() const
{
	return this->m_initialOriginTransform;
}

Transform3D CesiumGlobe::get_initial_tx_engine_to_ecef() const
{
	return this->m_initialOriginTransform.inverse();
}

Vector3 CesiumGlobe::get_global_surface_position(const Vector3& cameraPos, const Vector3& cameraDirection)
{
	//The center position translated in the Z axis (given by the relative camera facing direction) by the radius
	ERR_PRINT("Global surface position not yet implemented!");
	return Vector3();
}

Vector3 CesiumGlobe::get_global_center_position()
{
	return this->get_global_position();
}

Vector3 CesiumGlobe::get_mouse_pos_ecef()
{
	Viewport* viewport = this->get_viewport();
	Camera3D* cam = viewport->get_camera_3d();
	const Vector2& mousePos = viewport->get_mouse_position();

	//Aim direction (project a ray)
	const Vector3& aimDirection = cam->project_ray_normal(mousePos);

	const EcefVector3& aimDirectionEcef = this->get_tx_engine_to_ecef().basis.xform(aimDirection);
	const EcefVector3& aimPositionEcef = this->get_tx_engine_to_ecef().xform(cam->get_global_position());

	//NYI: Digital Elevation Model (DEM)

	//We only take into account the bare surface
	return this->trace_ray_to_ellipsoid(aimPositionEcef, aimDirectionEcef);
	
}

Vector3 CesiumGlobe::get_ellipsoid_dimensions() const
{
	return CesiumMathUtils::from_glm_vec3(CesiumGeospatial::Ellipsoid::WGS84.getRadii());
}

Vector3 CesiumGlobe::ray_to_surface(const Vector3& origin, const Vector3& direction) const
{
	const EcefVector3& directionEcef = this->get_tx_engine_to_ecef().basis.xform(direction);
	const EcefVector3& positionEcef = this->get_tx_engine_to_ecef().xform(origin);
	return this->trace_ray_to_ellipsoid(positionEcef, directionEcef);
}

Basis CesiumGlobe::eus_at_ecef(const EcefVector3& ecef) const
{
	EcefVector3 up = ecef.normalized();
	EcefVector3 east = -up.cross(Vector3(0, 0, 1)).normalized();
	EcefVector3 south = east.cross(up);
	return Basis(east, up, south);
}


Vector3 CesiumGlobe::get_normal_at_surface_pos(const EcefVector3& ecef) const
{
	const CesiumGeospatial::Ellipsoid& wgs84 = CesiumGeospatial::Ellipsoid::WGS84;
	glm::dvec3 surfaceNormal = wgs84.geodeticSurfaceNormal(CesiumMathUtils::to_glm_dvec3(ecef));
	Vector3 gdNormal = CesiumMathUtils::from_glm_vec3(surfaceNormal);
	gdNormal = this->get_initial_tx_ecef_to_engine().xform(gdNormal);
	gdNormal.y *= -1; //Invert the Y Axis because otherwise we puke
	return gdNormal;
}

/// @brief Based off of https://en.wikipedia.org/wiki/Line%E2%80%93sphere_intersection
EcefVector3 CesiumGlobe::trace_ray_to_ellipsoid(const EcefVector3& origin, const EcefVector3& rayDirection) const
{
	real_t r = get_ellipsoid_dimensions().x;
	real_t b = rayDirection.dot(origin);
	real_t c = origin.dot(origin) - (r * r);
	real_t det = (b * b) - c;
	if (det < 0) {
		return Vector3(NAN, NAN, NAN);
	}
	real_t d = -b - Math::sqrt(det);

	Vector3 pf = origin + rayDirection * d;
	return pf;
}

void CesiumGlobe::_bind_methods()
{
	ClassDB::bind_method(D_METHOD("get_tx_engine_to_ecef"), &CesiumGlobe::get_tx_engine_to_ecef);
	ClassDB::bind_method(D_METHOD("get_tx_ecef_to_engine"), &CesiumGlobe::get_tx_ecef_to_engine);

	ClassDB::bind_method(D_METHOD("get_initial_tx_engine_to_ecef"), &CesiumGlobe::get_initial_tx_engine_to_ecef);
	ClassDB::bind_method(D_METHOD("get_initial_tx_ecef_to_engine"), &CesiumGlobe::get_initial_tx_ecef_to_engine);

	ClassDB::bind_method(D_METHOD("eus_at_ecef"), &CesiumGlobe::eus_at_ecef);
	ClassDB::bind_method(D_METHOD("get_global_center_position"), &CesiumGlobe::get_global_center_position);
	ClassDB::bind_method(D_METHOD("get_global_surface_position"), &CesiumGlobe::get_global_surface_position);
	ClassDB::bind_method(D_METHOD("get_mouse_pos_ecef"), &CesiumGlobe::get_mouse_pos_ecef);
	ClassDB::bind_method(D_METHOD("ray_to_surface", "origin", "direction"), &CesiumGlobe::ray_to_surface);
	ClassDB::bind_method(D_METHOD("get_ellipsoid_dimensions"), &CesiumGlobe::get_ellipsoid_dimensions);
	ClassDB::bind_method(D_METHOD("get_normal_at_surface_pos"), &CesiumGlobe::get_normal_at_surface_pos);
}
