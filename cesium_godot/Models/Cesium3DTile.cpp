#include "Cesium3DTile.h"
#include "CesiumGltf/Model.h"
#include "CesiumGltf/PropertyTable.h"
#include "Models/TileMetadata.h"
#include "Utils/CesiumMathUtils.h"
#include "glm/ext/vector_double3.hpp"
#if defined(CESIUM_GD_EXT)
#include "godot_cpp/classes/collision_shape3d.hpp"
#include "godot_cpp/classes/concave_polygon_shape3d.hpp"
#include "godot_cpp/classes/mesh.hpp"
#include "godot_cpp/classes/static_body3d.hpp"
#include "godot_cpp/core/error_macros.hpp"
#include "godot_cpp/variant/packed_vector3_array.hpp"
#include "godot_cpp/variant/vector3.hpp"
#include "godot_cpp/classes/mesh.hpp"
#else
#include "scene/3d/physics/static_body_3d.h"
#include "scene/resources/3d/concave_polygon_shape_3d.h"
#include "scene/3d/physics/collision_shape_3d.h"
#endif
#include "CesiumGltf/PropertyTableView.h"
#include "CesiumGltf/Model.h"
#include <cstdint>
#include <vector>


void Cesium3DTile::_ready() {
	// Override to delete behavior
}

void Cesium3DTile::generate_tile_collision() {
	// Get our static body and add it as a child of the mesh
	StaticBody3D* staticBody = Object::cast_to<StaticBody3D>(this->create_collision_node_custom_trimesh());
	ERR_FAIL_NULL_MSG(staticBody, "Unable to generate tile collision, failed to create the tile's shape");
	staticBody->set_name(String(this->get_name()) + "_col");

	this->add_child(staticBody, true);
	// Set all the owners
	Node* owner = this->get_owner();
	if (owner == nullptr) return;
	
	CollisionShape3D* collisionShape = Object::cast_to<CollisionShape3D>(staticBody->get_child(0));
	staticBody->set_owner(owner);
	collisionShape->set_owner(owner);
}

void Cesium3DTile::add_metadata(const CesiumGltf::Model* model, const CesiumGltf::ExtensionModelExtStructuralMetadata* metadata) {
	if (metadata == nullptr) return;
	const std::vector<CesiumGltf::PropertyTable>& tables = metadata->propertyTables;

	// Reserve our own copy of the data tables
	this->m_metadata.init(tables.size());	

	for (size_t i = 0; i < tables.size(); i++) {
		const CesiumGltf::PropertyTable& table = tables[i];	
		CesiumGltf::PropertyTableView view(*model, table);
		if (view.status() != CesiumGltf::PropertyTableViewStatus::Valid) {
			// TODO: Save it but mark it as failed to load
			continue;
		}
		// Initialize the dictionary
		this->m_metadata.add_table(view);
		
	}
}

Ref<ConcavePolygonShape3D> Cesium3DTile::create_trimesh_shape_inverse_winding()  {
	// const auto& faces = this->get_mesh()->get_faces();
	// if (faces.size() == 0) {
	// 	return Ref<ConcavePolygonShape3D>();
	// }

	// PackedVector3Array facePoints;
	// facePoints.resize(faces.size());

	// for (int i = 0; i < facePoints.size(); i += 3) {
	// 	// Let's reverse it
	// 	facePoints.set(i, faces.get(i + 2));
	// 	facePoints.set(i + 1, faces.get(i + 1));
	// 	facePoints.set(i + 2, faces.get(i));
	// }

	Ref<ConcavePolygonShape3D> shape = memnew(ConcavePolygonShape3D);
	// shape->set_faces(facePoints);
	return shape;
}

Node* Cesium3DTile::create_collision_node_custom_trimesh() {
	Ref<ConcavePolygonShape3D> shape = this->create_trimesh_shape_inverse_winding();
	if (shape.is_null()) {
		return nullptr;
	}
	
	StaticBody3D* staticBody = memnew(StaticBody3D);
	CollisionShape3D* collisionShape = memnew(CollisionShape3D);

	collisionShape->set_shape(shape);
	staticBody->add_child(collisionShape, true);
	return staticBody;
}

const Dictionary& Cesium3DTile::get_metadata_table(int32_t index) const {
	return this->m_metadata.get_table(index);
}

int32_t Cesium3DTile::get_table_count() const {
	return this->m_metadata.get_table_count();
}

void Cesium3DTile::_bind_methods() {
    ClassDB::bind_method(D_METHOD("get_metadata_table", "index"), &Cesium3DTile::get_metadata_table);
    ClassDB::bind_method(D_METHOD("get_table_count"), &Cesium3DTile::get_table_count);
    ClassDB::bind_method(D_METHOD("generate_tile_collision"), &Cesium3DTile::generate_tile_collision);
}
