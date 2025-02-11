#include "CesiumGDGeoreference.h"
#include "CesiumGDTileset.h"
#include "../Utils/CesiumMathUtils.h"

double CesiumGDGeoreference::get_ecef_x() const
{
	return this->m_ecefPosition.x;
}

void CesiumGDGeoreference::set_ecef_x(double x)
{
	this->m_ecefPosition.x = x;
	this->move_origin();
}

double CesiumGDGeoreference::get_ecef_y() const
{
	return this->m_ecefPosition.y;
}

void CesiumGDGeoreference::set_ecef_y(double y)
{
	this->m_ecefPosition.y = y;
	this->move_origin();
}

double CesiumGDGeoreference::get_ecef_z() const
{
	return this->m_ecefPosition.z;
}

void CesiumGDGeoreference::set_ecef_z(double z)
{
	this->m_ecefPosition.z = z;
	this->move_origin();
}

real_t CesiumGDGeoreference::get_scale_factor() const
{
	return this->m_scaleFactor;
}

void CesiumGDGeoreference::set_scale_factor(real_t factor)
{
	this->m_scaleFactor = factor;
}

glm::dvec3 CesiumGDGeoreference::get_ecef_position() const
{
	return this->m_ecefPosition;
}

void CesiumGDGeoreference::move_origin()
{
	if (!this->m_shouldUpdateOrigin) return;
	// Translate the node by the ecef position...
	// Get the engine pos of the ecef position
	Vector3 enginePos = CesiumMathUtils::from_glm_vec3(this->m_ecefPosition);
	enginePos = this->get_tx_ecef_to_engine().xform(enginePos);
	// Take the current origin and subtract enginePos from that (dest - source)
	Vector3 currOrigin = this->get_global_position();
	this->set_global_position(currOrigin - enginePos);
}

void CesiumGDGeoreference::set_should_update_origin(bool updateOrigin)
{
	this->m_shouldUpdateOrigin = updateOrigin;
}

bool CesiumGDGeoreference::get_should_update_origin() const
{
	return this->m_shouldUpdateOrigin;
}

void CesiumGDGeoreference::_bind_methods()
{
	ClassDB::bind_method(D_METHOD("get_ecef_x"), &CesiumGDGeoreference::get_ecef_x);
	ClassDB::bind_method(D_METHOD("set_ecef_x", "ecefX"), &CesiumGDGeoreference::set_ecef_x);
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "ecefX"), "set_ecef_x", "get_ecef_x");

	ClassDB::bind_method(D_METHOD("get_ecef_y"), &CesiumGDGeoreference::get_ecef_y);
	ClassDB::bind_method(D_METHOD("set_ecef_y", "ecefY"), &CesiumGDGeoreference::set_ecef_y);
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "ecefY"), "set_ecef_y", "get_ecef_y");

	ClassDB::bind_method(D_METHOD("get_ecef_z"), &CesiumGDGeoreference::get_ecef_z);
	ClassDB::bind_method(D_METHOD("set_ecef_z", "ecefZ"), &CesiumGDGeoreference::set_ecef_z);
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "ecefZ"), "set_ecef_z", "get_ecef_z");

	ClassDB::bind_method(D_METHOD("get_scale_factor"), &CesiumGDGeoreference::get_scale_factor);
	ClassDB::bind_method(D_METHOD("set_scale_factor", "scale_factor"), &CesiumGDGeoreference::set_scale_factor);
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "scale_factor"), "set_scale_factor", "get_scale_factor");

}
