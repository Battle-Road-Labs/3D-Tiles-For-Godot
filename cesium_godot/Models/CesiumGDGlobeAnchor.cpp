#include "CesiumGDGlobeAnchor.h"
#include "Models/CesiumGlobe.h"
#include "Utils/CesiumMathUtils.h"

#include <glm/ext/vector_double3.hpp>
#include <glm/geometric.hpp>

#if defined(CESIUM_GD_EXT)
#include <godot_cpp/classes/engine.hpp>
#include <godot_cpp/core/error_macros.hpp>
#elif defined(CESIUM_GD_MODULE)
#include "core/error/error_macros.h"
#endif

static constexpr double GA_A = 6378137.0;
static constexpr double GA_B = 6.3567523e6;
static constexpr double GA_E_SQR = (GA_A * GA_A - GA_B * GA_B) / (GA_A * GA_A);
static constexpr double GA_EP_SQR = (GA_A * GA_A - GA_B * GA_B) / (GA_B * GA_B);

static glm::dvec3 s_lla_to_ecef(double lat_deg, double lon_deg, double alt_m) {
	const double lat = Math::deg_to_rad(lat_deg);
	const double lon = Math::deg_to_rad(lon_deg);
	const double clat = cos(lat), slat = sin(lat);
	const double clon = cos(lon), slon = sin(lon);
	const double N = GA_A / sqrt(1.0 - GA_E_SQR * slat * slat);
	return {
		(N + alt_m) * clat * clon,
		(N + alt_m) * clat * slon,
		(N * (1.0 - GA_E_SQR) + alt_m) * slat
	};
}

static glm::dvec3 s_ecef_to_lla(const glm::dvec3& ecef) {
	const double x = ecef.x, y = ecef.y, z = ecef.z;
	const double p = sqrt(x * x + y * y);
	const double theta = atan2(GA_A * z, GA_B * p);
	const double lon = atan2(y, x);
	const double lat = atan2(
		z + GA_EP_SQR * GA_B * pow(sin(theta), 3.0),
		p - GA_E_SQR * GA_A * pow(cos(theta), 3.0));
	const double sinLat = sin(lat);
	const double N = GA_A / sqrt(1.0 - GA_E_SQR * sinLat * sinLat);
	const double alt = (p / cos(lat)) - N;
	return { Math::rad_to_deg(lat), Math::rad_to_deg(lon), alt };
}

void CesiumGDGlobeAnchor::sync_ecef_from_lla() {
	m_ecefPosition = s_lla_to_ecef(m_latitude, m_longitude, m_altitude);
}

void CesiumGDGlobeAnchor::sync_lla_from_ecef() {
	const glm::dvec3 lla = s_ecef_to_lla(m_ecefPosition);
	m_latitude = lla.x;
	m_longitude = lla.y;
	m_altitude = lla.z;
}

CesiumGeoreference* CesiumGDGlobeAnchor::resolve_georeference() const {
	if (m_georeference) return m_georeference;

	Node* current = get_parent();
	while (current) {
		if (CesiumGeoreference* gr = Object::cast_to<CesiumGeoreference>(current)) {
			return gr;
		}

		for (int i = 0; i < current->get_child_count(); ++i) {
			if (CesiumGeoreference* gr = Object::cast_to<CesiumGeoreference>(current->get_child(i))) {
				return gr;
			}
		}

		current = current->get_parent();
	}

	return nullptr;
}

void CesiumGDGlobeAnchor::apply_globe_transform() {
	if (m_applyingTransform) {
		return;
	}

	CesiumGeoreference* georef = m_georeference ? m_georeference : resolve_georeference();
	if (!georef || !is_inside_tree()) {
		return;
	}

	m_applyingTransform = true;

	const Vector3 ecefPos = CesiumMathUtils::from_glm_vec3(m_ecefPosition);
	Vector3 worldPos = georef->get_tx_ecef_to_engine().xform(ecefPos);
	if (georef->get_origin_type_raw() == CesiumGeoreference::OriginType::CartographicOrigin) {
		const glm::dvec3 engineOrigin = CesiumMathUtils::ecef_to_engine(georef->get_ecef_position());
		worldPos -= CesiumMathUtils::from_glm_vec3(engineOrigin);
	}

	if (m_adjustOrientationToGlobe && glm::length(m_ecefPosition) > 1.0) {
		const Basis eusBasisEcef = georef->eus_at_ecef(ecefPos);
		const Basis engineBasis = georef->get_tx_ecef_to_engine().basis * eusBasisEcef;

		const Vector3 scale = get_scale();
		set_global_transform(Transform3D(engineBasis.scaled(scale), worldPos));
	} else {
		set_global_position(worldPos);
	}

	m_applyingTransform = false;
}

void CesiumGDGlobeAnchor::_ready() {
	m_georeference = resolve_georeference();
	if (!m_georeference) {
		WARN_PRINT("[CesiumGlobeAnchor] No CesiumGeoreference found");
		return;
	}

	m_lastGeoreferenceEcef = m_georeference->get_ecef_position();
	m_hasLastGeoreferenceEcef = true;
	set_process_internal(true);
	
	if (m_detectTransformChanges) {
		set_notify_transform(true);
	}

	apply_globe_transform();
}

void CesiumGDGlobeAnchor::_notification(int p_what) {
	if (p_what == NOTIFICATION_INTERNAL_PROCESS) {
		CesiumGeoreference* georef = m_georeference ? m_georeference : resolve_georeference();
		if (!georef) {
			return;
		}

		const glm::dvec3& currentGeoreferenceEcef = georef->get_ecef_position();
		if (!m_hasLastGeoreferenceEcef || currentGeoreferenceEcef != m_lastGeoreferenceEcef) {
			m_lastGeoreferenceEcef = currentGeoreferenceEcef;
			m_hasLastGeoreferenceEcef = true;
			apply_globe_transform();
		}

	} else if (p_what == NOTIFICATION_TRANSFORM_CHANGED) {
		if (!m_detectTransformChanges || m_applyingTransform) {
			return;
		}

		sync_from_engine_transform();
	}
}

double CesiumGDGlobeAnchor::get_latitude() const {
	return m_latitude;
}

double CesiumGDGlobeAnchor::get_longitude() const {
	return m_longitude;
}

double CesiumGDGlobeAnchor::get_altitude() const {
	return m_altitude;
}

void CesiumGDGlobeAnchor::set_latitude(double lat) {
	m_latitude = lat;
	sync_ecef_from_lla();
	apply_globe_transform();
}

void CesiumGDGlobeAnchor::set_longitude(double lon) {
	m_longitude = lon;
	sync_ecef_from_lla();
	apply_globe_transform();
}

void CesiumGDGlobeAnchor::set_altitude(double alt) {
	m_altitude = alt;
	sync_ecef_from_lla();
	apply_globe_transform();
}

double CesiumGDGlobeAnchor::get_ecef_x() const {
	return m_ecefPosition.x;
}

double CesiumGDGlobeAnchor::get_ecef_y() const {
	return m_ecefPosition.y;
}

double CesiumGDGlobeAnchor::get_ecef_z() const {
	return m_ecefPosition.z;
}

void CesiumGDGlobeAnchor::set_ecef_x(double x) {
	m_ecefPosition.x = x;
	sync_lla_from_ecef();
	apply_globe_transform();
}

void CesiumGDGlobeAnchor::set_ecef_y(double y) {
	m_ecefPosition.y = y;
	sync_lla_from_ecef();
	apply_globe_transform();
}

void CesiumGDGlobeAnchor::set_ecef_z(double z) {
	m_ecefPosition.z = z;
	sync_lla_from_ecef();
	apply_globe_transform();
}

bool CesiumGDGlobeAnchor::get_adjust_orientation_to_globe() const {
	return m_adjustOrientationToGlobe;
}

void CesiumGDGlobeAnchor::set_adjust_orientation_to_globe(bool adjust) {
	m_adjustOrientationToGlobe = adjust;
	apply_globe_transform();
}

bool CesiumGDGlobeAnchor::get_detect_transform_changes() const {
	return m_detectTransformChanges;
}

void CesiumGDGlobeAnchor::set_detect_transform_changes(bool detect) {
	m_detectTransformChanges = detect;
	set_notify_transform(detect);
}

CesiumGeoreference* CesiumGDGlobeAnchor::get_georeference() const {
	return m_georeference;
}

void CesiumGDGlobeAnchor::set_georeference(CesiumGeoreference* georeference) {
	m_georeference = georeference;
	m_hasLastGeoreferenceEcef = false;
	apply_globe_transform();
}

void CesiumGDGlobeAnchor::move_to_lla(double lat, double lon, double alt) {
	m_latitude = lat;
	m_longitude = lon;
	m_altitude = alt;
	sync_ecef_from_lla();
	apply_globe_transform();
}

void CesiumGDGlobeAnchor::move_to_ecef(double x, double y, double z) {
	m_ecefPosition = { x, y, z };
	sync_lla_from_ecef();
	apply_globe_transform();
}

void CesiumGDGlobeAnchor::sync_to_globe() {
	apply_globe_transform();
}

void CesiumGDGlobeAnchor::sync_from_engine_transform() {
	CesiumGeoreference* georef = m_georeference ? m_georeference : resolve_georeference();
	if (!georef) {
		return;
	}

	const Vector3 worldPos = get_global_position();
	Vector3 ecefInput = worldPos;
	if (georef->get_origin_type_raw() == CesiumGeoreference::OriginType::CartographicOrigin) {
		const glm::dvec3 engineOrigin = CesiumMathUtils::ecef_to_engine(georef->get_ecef_position());
		ecefInput += CesiumMathUtils::from_glm_vec3(engineOrigin);
	}

	const Vector3 ecefAbsolute = georef->get_tx_engine_to_ecef().xform(ecefInput);
	m_ecefPosition = CesiumMathUtils::to_glm_dvec3(ecefAbsolute);
	sync_lla_from_ecef();
}

void CesiumGDGlobeAnchor::_bind_methods() {
	ClassDB::bind_method(D_METHOD("get_georeference"), &CesiumGDGlobeAnchor::get_georeference);
	ClassDB::bind_method(D_METHOD("set_georeference", "georeference"), &CesiumGDGlobeAnchor::set_georeference);
	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "georeference", PROPERTY_HINT_NODE_TYPE, "CesiumGeoreference"), "set_georeference", "get_georeference");

	ClassDB::bind_method(D_METHOD("get_latitude"), &CesiumGDGlobeAnchor::get_latitude);
	ClassDB::bind_method(D_METHOD("set_latitude", "lat"), &CesiumGDGlobeAnchor::set_latitude);
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "latitude", PROPERTY_HINT_RANGE, "-90,90,0.000001"), "set_latitude", "get_latitude");

	ClassDB::bind_method(D_METHOD("get_longitude"), &CesiumGDGlobeAnchor::get_longitude);
	ClassDB::bind_method(D_METHOD("set_longitude", "lon"), &CesiumGDGlobeAnchor::set_longitude);
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "longitude", PROPERTY_HINT_RANGE, "-180,180,0.000001"), "set_longitude", "get_longitude");

	ClassDB::bind_method(D_METHOD("get_altitude"), &CesiumGDGlobeAnchor::get_altitude);
	ClassDB::bind_method(D_METHOD("set_altitude", "alt"), &CesiumGDGlobeAnchor::set_altitude);
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "altitude"), "set_altitude", "get_altitude");

	ClassDB::bind_method(D_METHOD("get_ecef_x"), &CesiumGDGlobeAnchor::get_ecef_x);
	ClassDB::bind_method(D_METHOD("set_ecef_x", "x"), &CesiumGDGlobeAnchor::set_ecef_x);
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "ecef_x", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_EDITOR), "set_ecef_x", "get_ecef_x");

	ClassDB::bind_method(D_METHOD("get_ecef_y"), &CesiumGDGlobeAnchor::get_ecef_y);
	ClassDB::bind_method(D_METHOD("set_ecef_y", "y"), &CesiumGDGlobeAnchor::set_ecef_y);
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "ecef_y", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_EDITOR), "set_ecef_y", "get_ecef_y");

	ClassDB::bind_method(D_METHOD("get_ecef_z"), &CesiumGDGlobeAnchor::get_ecef_z);
	ClassDB::bind_method(D_METHOD("set_ecef_z", "z"), &CesiumGDGlobeAnchor::set_ecef_z);
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "ecef_z", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_EDITOR), "set_ecef_z", "get_ecef_z");

	ClassDB::bind_method(D_METHOD("get_adjust_orientation_to_globe"), &CesiumGDGlobeAnchor::get_adjust_orientation_to_globe);
	ClassDB::bind_method(D_METHOD("set_adjust_orientation_to_globe", "adjust"), &CesiumGDGlobeAnchor::set_adjust_orientation_to_globe);
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "adjust_orientation_to_globe"), "set_adjust_orientation_to_globe", "get_adjust_orientation_to_globe");

	ClassDB::bind_method(D_METHOD("get_detect_transform_changes"), &CesiumGDGlobeAnchor::get_detect_transform_changes);
	ClassDB::bind_method(D_METHOD("set_detect_transform_changes", "detect"), &CesiumGDGlobeAnchor::set_detect_transform_changes);
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "detect_transform_changes"), "set_detect_transform_changes", "get_detect_transform_changes");

	ClassDB::bind_method(D_METHOD("move_to_lla", "latitude", "longitude", "altitude"), &CesiumGDGlobeAnchor::move_to_lla);
	ClassDB::bind_method(D_METHOD("move_to_ecef", "x", "y", "z"), &CesiumGDGlobeAnchor::move_to_ecef);
	ClassDB::bind_method(D_METHOD("sync_to_globe"), &CesiumGDGlobeAnchor::sync_to_globe);
	ClassDB::bind_method(D_METHOD("sync_from_engine_transform"), &CesiumGDGlobeAnchor::sync_from_engine_transform);
}
