#ifndef CESIUM_GD_GEOREFERENCE_H
#define CESIUM_GD_GEOREFERENCE_H

#if defined(CESIUM_GD_EXT)
#include <godot_cpp/classes/node3d.hpp>
using namespace godot;
#elif defined(CESIUM_GD_MODULE)
#include "scene/3d/node_3d.h"
#endif

#include "CesiumGlobe.h"
#include <glm/ext/vector_double3.hpp>

class CesiumGDTileset;

class CesiumGDGeoreference : public CesiumGlobe {
	GDCLASS(CesiumGDGeoreference, CesiumGlobe)
public:
#pragma region Editor methods
	double get_ecef_x() const;

	void set_ecef_x(double x);

	double get_ecef_y() const;

	void set_ecef_y(double y);

	double get_ecef_z() const;

	void set_ecef_z(double z);

	real_t get_scale_factor() const;

	void set_scale_factor(real_t factor);

#pragma endregion

	glm::dvec3 get_ecef_position() const;

	void move_origin();

	void set_should_update_origin(bool updateOrigin);

	bool get_should_update_origin() const;

private:
	glm::dvec3 m_ecefPosition{};

	real_t m_scaleFactor;

	bool m_shouldUpdateOrigin = false;

protected:
	static void _bind_methods();
};


#endif // CESIUM_GD_GEOREFERENCE_H
