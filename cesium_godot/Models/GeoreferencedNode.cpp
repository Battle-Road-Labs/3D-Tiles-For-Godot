#include "GeoreferencedNode.h"
#include "Models/CesiumGlobe.h"
#include "Utils/CesiumMathUtils.h"
#include "glm/ext/vector_double3.hpp"
#if defined(CESIUM_GD_EXT)
#include "godot_cpp/classes/engine.hpp"
#include "godot_cpp/classes/tile_set.hpp"
#include "godot_cpp/core/print_string.hpp"
#else
#endif


void GeoreferencedMesh::_ready() {
	if (Engine::get_singleton()->is_editor_hint()) return;

	// Use the georeference's origin to apply the difference in engine coords
	Vector3 ecefOrigin = CesiumMathUtils::from_glm_vec3(this->get_georeference()->get_ecef_position());
	Vector3 objectPos = this->get_georeference()->get_tx_engine_to_ecef().xform(this->get_global_position());
	this->set_ecef_position(objectPos + ecefOrigin);
}

void GeoreferencedMesh::apply_position_on_globe(const glm::dvec3& engineOrigin) {
	glm::dvec3 globalPos = this->m_originalPosition - engineOrigin;
	this->set_global_position(CesiumMathUtils::from_glm_vec3(globalPos));
}

const glm::dvec3& GeoreferencedMesh::get_original_position() const {
	return this->m_originalPosition;
}

void GeoreferencedMesh::set_original_position(const glm::dvec3& position) {
	this->m_originalPosition = position;
}

void GeoreferencedMesh::set_engine_position(const Vector3& position) {
	const CesiumGeoreference* georef = this->m_tileset->get_georeference_node();
	this->m_originalPosition = CesiumMathUtils::to_glm_dvec3(position);
	glm::dvec3 engineOrigin = CesiumMathUtils::ecef_to_engine(georef->get_ecef_position());
	this->apply_position_on_globe(engineOrigin);
}

void GeoreferencedMesh::set_ecef_position(const Vector3& position) {
	const CesiumGeoreference* georef = this->m_tileset->get_georeference_node();
	Vector3 enginePos = georef->get_tx_ecef_to_engine().xform(position);
	
	this->m_originalPosition = CesiumMathUtils::to_glm_dvec3(enginePos);
	glm::dvec3 engineOrigin = CesiumMathUtils::ecef_to_engine(georef->get_ecef_position());
	this->apply_position_on_globe(engineOrigin);
}


Vector3 GeoreferencedMesh::get_ecef_position() const {
	Vector3 originalPos = CesiumMathUtils::from_glm_vec3(this->m_originalPosition);
	return this->m_tileset->get_georeference_node()->get_tx_engine_to_ecef().xform(originalPos);
}

void GeoreferencedMesh::set_tileset(Cesium3DTileset* tileset) {
	this->m_tileset = tileset;
}


void GeoreferencedMesh::set_tileset_no_reparent(Cesium3DTileset* tileset) {
	this->m_tileset = tileset;
}

Vector3 GeoreferencedMesh::get_engine_position() const {
	return CesiumMathUtils::from_glm_vec3(this->m_originalPosition);
}


Cesium3DTileset* GeoreferencedMesh::get_tileset() const {
	return this->m_tileset;
}

CesiumGeoreference* GeoreferencedMesh::get_georeference() const {
	return this->m_tileset->get_georeference_node();
}


void GeoreferencedMesh::_bind_methods() {
    ClassDB::bind_method(D_METHOD("get_engine_position"), &GeoreferencedMesh::get_engine_position);
    ClassDB::bind_method(D_METHOD("set_engine_position", "position"), &GeoreferencedMesh::set_engine_position);
    ClassDB::bind_method(D_METHOD("get_ecef_position"), &GeoreferencedMesh::get_ecef_position);
    ClassDB::bind_method(D_METHOD("set_ecef_position", "position"), &GeoreferencedMesh::set_ecef_position);
    ClassDB::bind_method(D_METHOD("set_tileset", "tileset"), &GeoreferencedMesh::set_tileset);
    ClassDB::bind_method(D_METHOD("get_tileset"), &GeoreferencedMesh::get_tileset);
    ClassDB::bind_method(D_METHOD("get_georeference"), &GeoreferencedMesh::get_georeference);
	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "tileset", PropertyHint::PROPERTY_HINT_NODE_TYPE, "Cesium3DTileset"), "set_tileset", "get_tileset");
}

