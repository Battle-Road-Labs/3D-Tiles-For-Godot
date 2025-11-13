#include "CesiumGlobe.h"
#include "Models/CesiumGDTileset.h"
#include "Utils/AssetManipulation.h"
#include "glm/ext/vector_double3.hpp"
#include <cstdint>

#if defined(CESIUM_GD_EXT)
#include <godot_cpp/classes/viewport.hpp>
#include <godot_cpp/classes/camera3d.hpp>
#include "godot_cpp/core/math.hpp"
#include "godot_cpp/variant/vector3.hpp"
#else
#include "scene/main/viewport.h"
#include "scene/3d/camera_3d.h"
#endif


#include "CesiumGeospatial/Ellipsoid.h"
#include "../Utils/CesiumMathUtils.h"


// Use WSG84, cesium stopped makig that ellipsoid a constexpr, so we'll hardcode it here
constexpr double WGS84_RADIUS = 6378137.0;

constexpr double WGS84_B = 6.3567523E6;
constexpr double WGS84_BSQR = WGS84_B * WGS84_B;
constexpr double WGS84_R_SQR = WGS84_RADIUS * WGS84_RADIUS;
constexpr double WGS84_E_PRIME_SQR = (WGS84_R_SQR - WGS84_BSQR) / WGS84_BSQR;	
constexpr double WGS84_E_SQR = (WGS84_R_SQR - WGS84_BSQR) / WGS84_R_SQR;

Transform3D CesiumGeoreference::get_tx_engine_to_ecef() const
{
	return this->get_global_transform().inverse();
}

Transform3D CesiumGeoreference::get_tx_ecef_to_engine() const
{
	return this->get_global_transform();
}

Transform3D CesiumGeoreference::get_initial_tx_ecef_to_engine() const
{
	return this->m_initialOriginTransform;
}

Transform3D CesiumGeoreference::get_initial_tx_engine_to_ecef() const
{
	return this->m_initialOriginTransform.inverse();
}

Vector3 CesiumGeoreference::get_global_surface_position(const Vector3& cameraPos, const Vector3& cameraDirection)
{
	//The center position translated in the Z axis (given by the relative camera facing direction) by the radius
	ERR_PRINT("Global surface position not yet implemented!");
	return Vector3();
}

Vector3 CesiumGeoreference::get_global_center_position()
{
	return this->get_global_position();
}

Vector3 CesiumGeoreference::get_mouse_pos_ecef()
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

Vector3 CesiumGeoreference::get_ellipsoid_dimensions() const
{
	return CesiumMathUtils::from_glm_vec3(CesiumGeospatial::Ellipsoid::WGS84.getRadii());
}

int CesiumGeoreference::get_origin_type() const {
	return static_cast<int32_t>(this->m_originType);
}

void CesiumGeoreference::set_origin_type(int type) {
	this->m_originType = static_cast<OriginType>(type);
}

double CesiumGeoreference::get_ecef_x() const
{
	return this->m_ecefPosition.x;
}

void CesiumGeoreference::set_ecef_x(double x)
{
	glm::dvec3 prevPos = this->m_ecefPosition;
	this->m_ecefPosition.x = x;
	this->move_origin(prevPos);
}

double CesiumGeoreference::get_ecef_y() const
{
	return this->m_ecefPosition.y;
}

void CesiumGeoreference::set_ecef_y(double y)
{
	glm::dvec3 prevPos = this->m_ecefPosition;
	this->m_ecefPosition.y = y;
	this->move_origin(prevPos);
}

double CesiumGeoreference::get_ecef_z() const
{
	return this->m_ecefPosition.z;
}

void CesiumGeoreference::set_ecef_z(double z)
{
	glm::dvec3 prevPos = this->m_ecefPosition;
	this->m_ecefPosition.z = z;
	this->move_origin(prevPos);
}

real_t CesiumGeoreference::get_scale_factor() const
{
	return this->m_scaleFactor;
}

void CesiumGeoreference::set_scale_factor(real_t factor)
{
	this->m_scaleFactor = factor;
}

const glm::dvec3& CesiumGeoreference::get_ecef_position() const
{
	return this->m_ecefPosition;
}


void CesiumGeoreference::set_latitude(double lat) {
	glm::dvec3 lla = this->get_lla();
	lla.x = lat;
	this->update_ecef_with_lla(lla);	
}

void CesiumGeoreference::set_longitude(double longitude) {
	glm::dvec3 lla = this->get_lla();
	lla.y = longitude;
	this->update_ecef_with_lla(lla);		
}

void CesiumGeoreference::set_altitude(double alt) {
	glm::dvec3 lla = this->get_lla();
	lla.z = alt;
	this->update_ecef_with_lla(lla);
}

double CesiumGeoreference::get_latitude() const {
	return this->get_lla().x;
}

double CesiumGeoreference::get_longitude() const {
	return this->get_lla().y;
}

double CesiumGeoreference::get_altitude() const {
	return this->get_lla().z;
}

/// @brief All calculations are based off of https://en.wikipedia.org/wiki/Geographic_coordinate_conversion
/// using Ferrari's method to avoid iterations with Newton's method and all that stuff
glm::dvec3 CesiumGeoreference::get_lla() const {
	// Take ECEF and represent it as lla
	const double& x = this->m_ecefPosition.x;
	const double& y = this->m_ecefPosition.y;
	const double& z = this->m_ecefPosition.z;

	
	double p = Math::sqrt((x * x) + (y * y));
	double theta = Math::atan2(WGS84_RADIUS * z, WGS84_B * p);	

	double longitude = Math::atan2(y, x);

	// This was a tough one
	double lat = Math::atan2( (z + WGS84_E_PRIME_SQR * WGS84_B * Math::pow(Math::sin(theta), 3)), (p - WGS84_E_SQR * WGS84_RADIUS * Math::pow(Math::cos(theta), 3)));
	double N = WGS84_RADIUS / (Math::sqrt(1 - (WGS84_E_SQR * Math::pow(Math::sin(lat), 2))));
	double m = (p / Math::cos(lat));
  
	double alt = m - N;
	return glm::dvec3(Math::rad_to_deg(lat), Math::rad_to_deg(longitude), alt);
}


void CesiumGeoreference::register_tileset_to_move_origin(Cesium3DTileset* tileset) {
	this->m_trackedTilesets.emplace_back(tileset);
}

/// @brief Approximation solution from Apple's pARk
/// https://developer.apple.com/library/archive/samplecode/pARk/Introduction/Intro.html
void CesiumGeoreference::update_ecef_with_lla(glm::dvec3 lla) {
	const double& lat = lla.x;
	const double& lon = lla.y;
	const double& alt = lla.z;
	
  double clat = cos(Math::deg_to_rad(lat));
  double slat = sin(Math::deg_to_rad(lat));
  double clon = cos(Math::deg_to_rad(lon));
  double slon = sin(Math::deg_to_rad(lon));	
  
	double N = WGS84_RADIUS / sqrt(1.0 - WGS84_E_SQR * slat * slat);

	this->m_ecefPosition.x = (N + alt) * clat * clon;
	this->m_ecefPosition.y = (N + alt) * clat * slon;
	this->m_ecefPosition.z = (N * (1 - WGS84_E_SQR) + alt) * slat;
}


void CesiumGeoreference::move_origin(glm::dvec3 previousPosition)
{
	// Move all the tilesets
	// const glm::dvec3 prevEnginePos = CesiumMathUtils::ecef_to_engine(previousPosition);
	const glm::dvec3 currEnginePos = CesiumMathUtils::ecef_to_engine(this->m_ecefPosition);
	for (Cesium3DTileset* tileset : this->m_trackedTilesets) {
		// Now we can recalculate the same positions
		tileset->move_origin(currEnginePos);
	}
}

void CesiumGeoreference::set_should_update_origin(bool updateOrigin)
{
	this->m_shouldUpdateOrigin = updateOrigin;
}

bool CesiumGeoreference::get_should_update_origin() const
{
	return this->m_shouldUpdateOrigin;
}


const glm::dvec3& CesiumGeoreference::get_original_origin_ecef() const {
	return this->m_originalEcefPosition;
}

Vector3 CesiumGeoreference::ray_to_surface(const Vector3& origin, const Vector3& direction) const
{
	const EcefVector3& directionEcef = this->get_tx_engine_to_ecef().basis.xform(direction);
	const EcefVector3& positionEcef = this->get_tx_engine_to_ecef().xform(origin);
	return this->trace_ray_to_ellipsoid(positionEcef, directionEcef);
}

Basis CesiumGeoreference::eus_at_ecef(const EcefVector3& ecef) const
{
	EcefVector3 up = ecef.normalized();
	EcefVector3 east = -up.cross(Vector3(0, 0, 1)).normalized();
	EcefVector3 south = east.cross(up);
	return Basis(east, up, south);
}


Vector3 CesiumGeoreference::get_normal_at_surface_pos(const EcefVector3& ecef) const
{
	const CesiumGeospatial::Ellipsoid& wgs84 = CesiumGeospatial::Ellipsoid::WGS84;
	glm::dvec3 surfaceNormal = wgs84.geodeticSurfaceNormal(CesiumMathUtils::to_glm_dvec3(ecef));
	Vector3 gdNormal = CesiumMathUtils::from_glm_vec3(surfaceNormal);
	gdNormal = this->get_initial_tx_ecef_to_engine().xform(gdNormal);
	return gdNormal;
}

/// @brief Based off of https://en.wikipedia.org/wiki/Line%E2%80%93sphere_intersection
EcefVector3 CesiumGeoreference::trace_ray_to_ellipsoid(const EcefVector3& origin, const EcefVector3& rayDirection) const
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

void CesiumGeoreference::_bind_methods()
{
	ClassDB::bind_method(D_METHOD("get_tx_engine_to_ecef"), &CesiumGeoreference::get_tx_engine_to_ecef);
	ClassDB::bind_method(D_METHOD("get_tx_ecef_to_engine"), &CesiumGeoreference::get_tx_ecef_to_engine);

	ClassDB::bind_method(D_METHOD("get_initial_tx_engine_to_ecef"), &CesiumGeoreference::get_initial_tx_engine_to_ecef);
	ClassDB::bind_method(D_METHOD("get_initial_tx_ecef_to_engine"), &CesiumGeoreference::get_initial_tx_ecef_to_engine);

	ClassDB::bind_method(D_METHOD("eus_at_ecef"), &CesiumGeoreference::eus_at_ecef);
	ClassDB::bind_method(D_METHOD("get_global_center_position"), &CesiumGeoreference::get_global_center_position);
	ClassDB::bind_method(D_METHOD("get_global_surface_position"), &CesiumGeoreference::get_global_surface_position);
	ClassDB::bind_method(D_METHOD("get_mouse_pos_ecef"), &CesiumGeoreference::get_mouse_pos_ecef);
	ClassDB::bind_method(D_METHOD("ray_to_surface", "origin", "direction"), &CesiumGeoreference::ray_to_surface);
	ClassDB::bind_method(D_METHOD("get_ellipsoid_dimensions"), &CesiumGeoreference::get_ellipsoid_dimensions);
	ClassDB::bind_method(D_METHOD("get_normal_at_surface_pos"), &CesiumGeoreference::get_normal_at_surface_pos);


	ClassDB::bind_method(D_METHOD("get_ecef_x"), &CesiumGeoreference::get_ecef_x);
	ClassDB::bind_method(D_METHOD("set_ecef_x", "ecefX"), &CesiumGeoreference::set_ecef_x);
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "ecefX"), "set_ecef_x", "get_ecef_x");

	ClassDB::bind_method(D_METHOD("get_ecef_y"), &CesiumGeoreference::get_ecef_y);
	ClassDB::bind_method(D_METHOD("set_ecef_y", "ecefY"), &CesiumGeoreference::set_ecef_y);
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "ecefY"), "set_ecef_y", "get_ecef_y");

	ClassDB::bind_method(D_METHOD("get_ecef_z"), &CesiumGeoreference::get_ecef_z);
	ClassDB::bind_method(D_METHOD("set_ecef_z", "ecefZ"), &CesiumGeoreference::set_ecef_z);
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "ecefZ"), "set_ecef_z", "get_ecef_z");

	ClassDB::bind_method(D_METHOD("get_scale_factor"), &CesiumGeoreference::get_scale_factor);
	ClassDB::bind_method(D_METHOD("set_scale_factor", "scale_factor"), &CesiumGeoreference::set_scale_factor);
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "scale_factor"), "set_scale_factor", "get_scale_factor");
		

	ClassDB::bind_method(D_METHOD("set_latitude", "latitude"), &CesiumGeoreference::set_latitude);
	ClassDB::bind_method(D_METHOD("get_latitude"), &CesiumGeoreference::get_latitude);
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "latitude"), "set_latitude", "get_latitude");


	// Longitude
	ClassDB::bind_method(D_METHOD("set_longitude", "longitude"), &CesiumGeoreference::set_longitude);
	ClassDB::bind_method(D_METHOD("get_longitude"), &CesiumGeoreference::get_longitude);
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "longitude"), "set_longitude", "get_longitude");

	// Altitude
	ClassDB::bind_method(D_METHOD("set_altitude", "altitude"), &CesiumGeoreference::set_altitude);
	ClassDB::bind_method(D_METHOD("get_altitude"), &CesiumGeoreference::get_altitude);
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "altitude"), "set_altitude", "get_altitude");
	
	
	ClassDB::bind_method(D_METHOD("get_origin_type"), &CesiumGeoreference::get_origin_type);
	ClassDB::bind_method(D_METHOD("set_origin_type", "data_source"), &CesiumGeoreference::set_origin_type);
	ADD_PROPERTY(PropertyInfo(Variant::INT, "origin_type", PROPERTY_HINT_ENUM, "Cartographic Origin,True Origin"), "set_origin_type", "get_origin_type");
	ClassDB::bind_integer_constant(get_class_static(), "OriginType", "CartographicOrigin", static_cast<int32_t>(OriginType::CartographicOrigin));
	ClassDB::bind_integer_constant(get_class_static(), "OriginType", "TrueOrigin", static_cast<int32_t>(OriginType::TrueOrigin));
}

void CesiumGeoreference::_enter_tree() {	
		this->m_initialOriginTransform = this->get_global_transform();
		this->m_originalEcefPosition = this->m_ecefPosition;
		this->set_rotation_degrees(Vector3(-90.0, 0.0, 0.0));
}
	
