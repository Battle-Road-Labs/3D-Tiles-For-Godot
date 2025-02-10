#include "AssetManipulation.h"

#include "../Models/CesiumGlobe.h"
#include "Models/CesiumGDRasterOverlay.h"
#include "Models/CesiumGDTileset.h"
#include "godot_cpp/classes/scene_tree.hpp"
#include "godot_cpp/core/error_macros.hpp"
#include "godot_cpp/core/memory.hpp"
#include "magic_enum.hpp"
#include <winnt.h>

const char* CESIUM_GLOBE_NAME = "CesiumGlobe";
const char* CESIUM_GLOBE_GEOREF = "CesiumGeoreference";
const char* CESIUM_TILESET_NAME = "CesiumGDTileset";

CesiumGlobe* Godot3DTiles::AssetManipulation::find_or_create_globe(Node3D* baseNode) {
  Node3D* root = get_root_of_edit_scene(baseNode);
	CesiumGlobe* globe = nullptr;
	int32_t count = root->get_child_count();
	for (int32_t i = 0; i < count; i++) {
		Node* child = root->get_child(i);
		CesiumGlobe* foundChild = Object::cast_to<CesiumGlobe>(child);
		if (foundChild != nullptr) {
			return foundChild;
		}
	}
	
	//Create a globe
	globe = memnew(CesiumGlobe);
	globe->set_name(CESIUM_GLOBE_NAME);
	globe->set_rotation_degrees(Vector3(-90.0, 0.0, 0.0));
	root->add_child(globe);
	globe->set_owner(root);
	return globe;
}

Node3D* Godot3DTiles::AssetManipulation::get_root_of_edit_scene(Node3D* baseNode) {
  return Object::cast_to<Node3D>(baseNode->get_tree()->get_edited_scene_root());
}


void Godot3DTiles::AssetManipulation::instantiate_tileset(Node3D* baseNode, int32_t tilesetType) {
	Node3D* root = get_root_of_edit_scene(baseNode);
	CesiumGDTileset* tileset = memnew(CesiumGDTileset);
	root->add_child(tileset);
	
	
	TilesetType actualType = static_cast<TilesetType>(tilesetType);
	CesiumGDRasterOverlay* rasterOverlay = nullptr;
	CesiumGDTileset* extraTileset = nullptr;
	
	constexpr int32_t cesiumWorldTerrainId = 1;
	constexpr int32_t bingMapsAerialWithLabelsId = 3;
	constexpr int32_t osmBuildingsId = 96188;
	constexpr int32_t bingRoadsId = 4;
	
	switch(actualType) {
		case Godot3DTiles::AssetManipulation::TilesetType::Blank:
			break;
		case Godot3DTiles::AssetManipulation::TilesetType::BingMapsAerialWithLabels:
			tileset->set_ion_asset_id(cesiumWorldTerrainId);
			rasterOverlay = memnew(CesiumGDRasterOverlay);
			rasterOverlay->set_asset_id(bingMapsAerialWithLabelsId);
			break;
		
		case Godot3DTiles::AssetManipulation::TilesetType::BingMapsRoads:
			tileset->set_ion_asset_id(cesiumWorldTerrainId);
			rasterOverlay = memnew(CesiumGDRasterOverlay);
			rasterOverlay->set_asset_id(bingRoadsId);
			break;
			
		case Godot3DTiles::AssetManipulation::TilesetType::OsmBuildings:
			tileset->set_ion_asset_id(cesiumWorldTerrainId);
			rasterOverlay = memnew(CesiumGDRasterOverlay);
			rasterOverlay->set_asset_id(bingMapsAerialWithLabelsId);
			extraTileset = memnew(CesiumGDTileset);
			extraTileset->set_ion_asset_id(osmBuildingsId);
			break;
		default:
			ERR_PRINT(String("Tileset type not implemented: ") + magic_enum::enum_name(actualType).data());
			break;	
	}

	if (extraTileset != nullptr) {
		root->add_child(extraTileset);
	}

	if (rasterOverlay != nullptr) {
		tileset->add_child(rasterOverlay);
	}
	
}


void Godot3DTiles::AssetManipulation::instantiate_dynamic_cam(Node3D* baseNode) {
	
}

