#include "CesiumFeaturesMetadata.h"

#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/packed_float32_array.hpp>

#include <Cesium3DTilesSelection/Tile.h>
#include <Cesium3DTilesSelection/TileContent.h>
#include <CesiumGltf/Model.h>
#include <CesiumGltf/PropertyTableView.h>

using namespace godot;
using namespace CesiumGltf;
using namespace Cesium3DTilesSelection;

CesiumFeaturesMetadata::CesiumFeaturesMetadata() {

}

Vector<float> CesiumFeaturesMetadata::get_property_values(const CesiumGltf::Model& model, const std::string& property_name) {
    Vector<float> values;

    const CesiumGltf::ExtensionModelExtStructuralMetadata* metadata = model.getExtension<CesiumGltf::ExtensionModelExtStructuralMetadata>();
    if (!metadata || metadata->propertyTables.empty()) {
        return values;
    }

    const CesiumGltf::PropertyTable& propertyTable = metadata->propertyTables[0];

    auto it = propertyTable.properties.find(property_name);
    if (it == propertyTable.properties.end()) {
        return values;
    }

    const PropertyTableProperty& property = it->second;
    int32_t bufferViewIndex = property.values;
    if (bufferViewIndex < 0 || bufferViewIndex >= model.bufferViews.size()) {
        return values;
    }

    const CesiumGltf::BufferView& bufferView = model.bufferViews[bufferViewIndex];
    const CesiumGltf::Buffer& buffer = model.buffers[bufferView.buffer];

    if (bufferView.byteOffset + bufferView.byteLength > buffer.cesium.data.size()) {
        return values;
    }

    const std::byte* data = &buffer.cesium.data[bufferView.byteOffset];

    int64_t count = propertyTable.count;
    values.resize(count);

    for (size_t idx = 0; idx < count; idx++) {
        float val = 0.0f;

        if (property_name == "year") {
            const uint32_t* ptr = reinterpret_cast<const uint32_t*>(data + idx * sizeof(uint32_t));
            val = static_cast<float>(*ptr);
        } else if (property_name == "height") {
            const float* ptr = reinterpret_cast<const float*>(data + idx * sizeof(float));
            val = *ptr;
        } else {
            ERR_PRINT("Unknown option");
        }

        values.write[idx] = val;
    }
    
    return values;
}

Vector<float> CesiumFeaturesMetadata::get_feature_ids_from_primitive(const CesiumGltf::MeshPrimitive& primitive, const CesiumGltf::Model& model) {
    Vector<float> featureIds;
    printf("primitive.attributes %" PRIu64  "\n", primitive.attributes.size());
    std::unordered_map<std::string, int32_t>::const_iterator it = primitive.attributes.find("_FEATURE_ID_0");
    if (it == primitive.attributes.end()) {
        return featureIds;
    }

    //printf("%d - %d", it->first, it->second);
    printf("\n %d \n", it->second);

    const CesiumGltf::Accessor& accessor = model.accessors[it->second];
    const CesiumGltf::BufferView& bufferView = model.bufferViews[accessor.bufferView];
    const CesiumGltf::Buffer& buffer = model.buffers[bufferView.buffer];

    if (bufferView.byteOffset + accessor.byteOffset >= buffer.cesium.data.size()) {
        return featureIds;
    }

    const std::byte* data = &buffer.cesium.data[bufferView.byteOffset + accessor.byteOffset];
    int64_t stride = bufferView.byteStride.value_or(accessor.computeByteSizeOfComponent());

    featureIds.resize(accessor.count);

    for (size_t i = 0; i < accessor.count; ++i) {
        float value = 0.0f;
        const std::byte* ptr = data + i * stride;
        value = static_cast<float>(*reinterpret_cast<const uint32_t*>(ptr));

        featureIds.write[i] = value;
    }
    printf("featureIds ", featureIds);
    return featureIds;
}

/*Dictionary CesiumFeaturesMetadata::get_property_table(const Tile* tile, const String& table_name) {
    return Dictionary();
}

TypedArray<String> CesiumFeaturesMetadata::list_available_properties(const Tile* tile) {
    return TypedArray<String>();
}*/

void CesiumFeaturesMetadata::_bind_methods() {
    //ClassDB::bind_method(D_METHOD("get_feature_ids_from_primitive", "primitive", "model"), &CesiumFeaturesMetadata::get_feature_ids_from_primitive);
}