#include "AssetManipulation.h"

#include "../Models/CesiumGlobe.h"
#include "godot_cpp/classes/scene_tree.hpp"

const char* CESIUM_GLOBE_NAME = "CesiumGlobe";
const char* CESIUM_GLOBE_GEOREF = "CesiumGeoreference";
const char* CESIUM_TILESET_NAME = "CesiumGDTileset";

CesiumGlobe* Godot3DTiles::AssetManipulation::find_or_create_globe(Node3D* baseNode) {
  Node3D* root = get_root_of_edit_scene(baseNode);
	CesiumGlobe* globe = nullptr;
	for (int32_t i = 0; i < root->get_child_count(); i++) {
		Node* child = root->get_child(i);
		CesiumGlobe* foundChild = Object::cast_to<CesiumGlobe>(child);
		if (foundChild != nullptr) {
			return foundChild;
		}
	}
	//Create a globe
	globe = memnew(CesiumGlobe);
	globe->set_name(CESIUM_GLOBE_NAME);
	root->add_child(globe);
	globe->_owner = root;
	return globe;
}

Node3D* Godot3DTiles::AssetManipulation::get_root_of_edit_scene(Node3D* baseNode) {
  return Object::cast_to<Node3D>(baseNode->get_tree()->get_edited_scene_root());
}

