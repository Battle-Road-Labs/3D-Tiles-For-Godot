#ifndef CESIUMFEATURESMETADATA_H
#define CESIUMFEATURESMETADATA_H

#include <godot_cpp/classes/node3d.hpp>
#include <godot_cpp/templates/vector.hpp>
#include <godot_cpp/variant/string.hpp>

#include <Cesium3DTilesSelection/Tile.h>
#include <CesiumGltf/Model.h>
#include <CesiumGltf/ExtensionModelExtStructuralMetadata.h>

using namespace godot;

class CesiumFeaturesMetadata : public godot::Node3D {
    GDCLASS(CesiumFeaturesMetadata, godot::Node3D);

protected:
    static void _bind_methods();

public:
    CesiumFeaturesMetadata();
    ~CesiumFeaturesMetadata() {};

    static Vector<float> get_property_values(const CesiumGltf::Model& model, const std::string& property_name);

    static Vector<float> get_feature_ids_from_primitive(const CesiumGltf::MeshPrimitive& primitive, const CesiumGltf::Model& model);

    //Dictionary get_property_table(const Cesium3DTilesSelection::Tile* tile, const String& table_name);

    //TypedArray<String> list_available_properties(const Cesium3DTilesSelection::Tile* tile);
 
};

#endif /* CESIUMFEATURESMETADATA_H */
