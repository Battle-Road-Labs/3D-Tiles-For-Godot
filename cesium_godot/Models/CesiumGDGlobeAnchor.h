#ifndef CESIUM_GLOBE_ANCHOR_H
#define CESIUM_GLOBE_ANCHOR_H

#include <glm/ext/vector_double3.hpp>

#if defined(CESIUM_GD_EXT)
#include <godot_cpp/classes/node3d.hpp>
using namespace godot;
#elif defined(CESIUM_GD_MODULE)
#include "scene/3d/node_3d.h"
#endif

class CesiumGeoreference;

class CesiumGDGlobeAnchor : public Node3D {
	GDCLASS(CesiumGDGlobeAnchor, Node3D)

public:
	void _ready() override;
	void _notification(int p_what);

	double get_latitude() const;
	void set_latitude(double lat);

	double get_longitude() const;
	void set_longitude(double lon);

	double get_altitude() const;
	void set_altitude(double alt);

	double get_ecef_x() const;
	void set_ecef_x(double x);

	double get_ecef_y() const;
	void set_ecef_y(double y);

	double get_ecef_z() const;
	void set_ecef_z(double z);

	bool get_adjust_orientation_to_globe() const;
	void set_adjust_orientation_to_globe(bool adjust);

	bool get_detect_transform_changes() const;
	void set_detect_transform_changes(bool detect);

	CesiumGeoreference* get_georeference() const;
	void set_georeference(CesiumGeoreference* georeference);

	void move_to_lla(double lat, double lon, double alt);

	void move_to_ecef(double x, double y, double z);

	void sync_to_globe();

	void sync_from_engine_transform();

private:
	void apply_globe_transform();
	CesiumGeoreference* resolve_georeference() const;

	void sync_ecef_from_lla();

	void sync_lla_from_ecef();

	glm::dvec3 m_ecefPosition{ 0.0, 0.0, 0.0 };

	double m_latitude = 0.0;
	double m_longitude = 0.0;
	double m_altitude = 0.0;

	bool m_adjustOrientationToGlobe = true;
	bool m_detectTransformChanges = false;

	bool m_applyingTransform = false;
	glm::dvec3 m_lastGeoreferenceEcef{ 0.0, 0.0, 0.0 };
	bool m_hasLastGeoreferenceEcef = false;

	CesiumGeoreference* m_georeference = nullptr;

protected:
	static void _bind_methods();
};

#endif // CESIUM_GLOBE_ANCHOR_H
