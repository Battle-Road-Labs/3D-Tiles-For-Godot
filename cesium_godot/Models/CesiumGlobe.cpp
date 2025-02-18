
#include "CesiumGlobe.h"
#include "Utils/AssetManipulation.h"
#include "godot_cpp/variant/vector3.hpp"
#include "missing_functions.hpp"
#include <cstdint>

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

int CesiumGlobe::get_origin_type() const {
	return static_cast<int32_t>(this->m_originType);
}

void CesiumGlobe::set_origin_type(int type) {
	this->m_originType = static_cast<OriginType>(type);
}

double CesiumGlobe::get_ecef_x() const
{
	return this->m_ecefPosition.x;
}

void CesiumGlobe::set_ecef_x(double x)
{
	this->m_ecefPosition.x = x;
	this->move_origin();
}

double CesiumGlobe::get_ecef_y() const
{
	return this->m_ecefPosition.y;
}

void CesiumGlobe::set_ecef_y(double y)
{
	this->m_ecefPosition.y = y;
	this->move_origin();
}

double CesiumGlobe::get_ecef_z() const
{
	return this->m_ecefPosition.z;
}

void CesiumGlobe::set_ecef_z(double z)
{
	this->m_ecefPosition.z = z;
	this->move_origin();
}

real_t CesiumGlobe::get_scale_factor() const
{
	return this->m_scaleFactor;
}

void CesiumGlobe::set_scale_factor(real_t factor)
{
	this->m_scaleFactor = factor;
}

glm::dvec3 CesiumGlobe::get_ecef_position() const
{
	return this->m_ecefPosition;
}

void CesiumGlobe::move_origin()
{
	if (!this->m_shouldUpdateOrigin || this->m_originType == OriginType::TrueOrigin) return;
	// Translate the node by the ecef position...
	// Get the engine pos of the ecef position
	Vector3 enginePos = CesiumMathUtils::from_glm_vec3(this->m_ecefPosition);
	enginePos = this->get_tx_ecef_to_engine().xform(enginePos);
	// Take the current origin and subtract enginePos from that (dest - source)
	Vector3 currOrigin = this->get_global_position();
	this->set_global_position(currOrigin - enginePos);
}

void CesiumGlobe::set_should_update_origin(bool updateOrigin)
{
	this->m_shouldUpdateOrigin = updateOrigin;
}

bool CesiumGlobe::get_should_update_origin() const
{
	return this->m_shouldUpdateOrigin;
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


	ClassDB::bind_method(D_METHOD("get_ecef_x"), &CesiumGlobe::get_ecef_x);
	ClassDB::bind_method(D_METHOD("set_ecef_x", "ecefX"), &CesiumGlobe::set_ecef_x);
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "ecefX"), "set_ecef_x", "get_ecef_x");

	ClassDB::bind_method(D_METHOD("get_ecef_y"), &CesiumGlobe::get_ecef_y);
	ClassDB::bind_method(D_METHOD("set_ecef_y", "ecefY"), &CesiumGlobe::set_ecef_y);
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "ecefY"), "set_ecef_y", "get_ecef_y");

	ClassDB::bind_method(D_METHOD("get_ecef_z"), &CesiumGlobe::get_ecef_z);
	ClassDB::bind_method(D_METHOD("set_ecef_z", "ecefZ"), &CesiumGlobe::set_ecef_z);
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "ecefZ"), "set_ecef_z", "get_ecef_z");

	ClassDB::bind_method(D_METHOD("get_scale_factor"), &CesiumGlobe::get_scale_factor);
	ClassDB::bind_method(D_METHOD("set_scale_factor", "scale_factor"), &CesiumGlobe::set_scale_factor);
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "scale_factor"), "set_scale_factor", "get_scale_factor");
	
	
	ClassDB::bind_method(D_METHOD("get_origin_type"), &CesiumGlobe::get_origin_type);
	ClassDB::bind_method(D_METHOD("set_origin_type", "data_source"), &CesiumGlobe::set_origin_type);
	ADD_PROPERTY(PropertyInfo(Variant::INT, "origin_type", PROPERTY_HINT_ENUM, "Cartographic Origin,True Origin"), "set_origin_type", "get_origin_type");
	BIND_ENUM_CONSTANT(static_cast<int64_t>(OriginType::CartographicOrigin));
	BIND_ENUM_CONSTANT(static_cast<int64_t>(OriginType::TrueOrigin));
}

void CesiumGlobe::_enter_tree() {	
		this->m_initialOriginTransform = this->get_global_transform();
		if (!is_editor_mode()) {
				return;
		} 
		
		this->set_rotation_degrees(Vector3(-90.0, 0.0, 0.0));
}
	
