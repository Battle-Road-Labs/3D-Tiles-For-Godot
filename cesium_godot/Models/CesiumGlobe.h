#ifndef CESIUM_GLOBE_H
#define CESIUM_GLOBE_H

#if defined(CESIUM_GD_EXT)
#include <godot_cpp/classes/object.hpp>
#include <godot_cpp/classes/node3d.hpp>
using namespace godot;
#elif defined(CESIUM_GD_MODULE)
#include "core/object/object.h"
#include "scene/3d/node_3d.h"
#endif

#include <glm/ext/vector_double3.hpp>

/// @brief small typedef for clarity
using EcefVector3 = Vector3;

/// @brief We define this in C++ to provide any utility methods from the Globe instance
class CesiumGlobe : public Node3D {
	GDCLASS(CesiumGlobe, Node3D)
public:

	enum class OriginType {
		CartographicOrigin, //Georeference
		TrueOrigin
	};

	CesiumGlobe() {
		this->m_initialOriginTransform = this->get_global_transform();
	};

	Transform3D get_tx_engine_to_ecef() const;

	Transform3D get_tx_ecef_to_engine() const;

	Transform3D get_initial_tx_ecef_to_engine() const;

	Transform3D get_initial_tx_engine_to_ecef() const;

	Vector3 get_global_surface_position(const Vector3& cameraPos, const Vector3& cameraDirection);

	Vector3 get_global_center_position();

	Vector3 get_mouse_pos_ecef();

	Vector3 get_ellipsoid_dimensions() const;

	Vector3 ray_to_surface(const Vector3& origin, const Vector3& direction) const;

	Basis eus_at_ecef(const EcefVector3& ecef) const;

	Vector3 get_normal_at_surface_pos(const EcefVector3& ecef) const;

	int get_origin_type() const;

	void set_origin_type(int type);

	double get_ecef_x() const;

	void set_ecef_x(double x);

	double get_ecef_y() const;

	void set_ecef_y(double y);

	double get_ecef_z() const;

	void set_ecef_z(double z);

	real_t get_scale_factor() const;

	void set_scale_factor(real_t factor);
	
	glm::dvec3 get_ecef_position() const;

	void move_origin();

	void set_should_update_origin(bool updateOrigin);

	bool get_should_update_origin() const;
	
	void _enter_tree() override;
	
private:
	EcefVector3 trace_ray_to_ellipsoid(const EcefVector3& origin, const EcefVector3& rayDirection) const;

	Transform3D m_initialOriginTransform;
	
	glm::dvec3 m_ecefPosition{};

	real_t m_scaleFactor = 1.0;

	bool m_shouldUpdateOrigin = false;
	
	OriginType m_originType = OriginType::CartographicOrigin;
	
protected:
	static void _bind_methods();

};

#endif // !CESIUM_GLOBE_H
