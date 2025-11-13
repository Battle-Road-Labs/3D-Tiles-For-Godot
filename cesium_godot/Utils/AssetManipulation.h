#pragma once

#include "Models/CesiumGDConfig.h"
#include "Models/CesiumGDCreditSystem.h"
#include "Models/CesiumGDTileset.h"
#if defined(CESIUM_GD_EXT)
#include "godot_cpp/classes/camera3d.hpp"
#include "godot_cpp/variant/array.hpp"
#include "godot_cpp/variant/string.hpp"
#include <godot_cpp/classes/node3d.hpp>
using namespace godot;
#else
#include "scene/main/node.h"
#endif

class CesiumGeoreference;


namespace Godot3DTiles::AssetManipulation {

  enum class TilesetType : int32_t {
    Blank,
    OsmBuildings,
    GooglePhotorealistic,
    BingMapsAerialWithLabels,
    BingMapsRoads
  };  


  void instantiate_tileset(Node* baseNode, int32_t assetId = 0, const String& assetType = "");
  
  void instantiate_dynamic_cam(Node* baseNode);
  
  CesiumGeoreference* find_or_create_globe(Node* baseNode);

  Node* get_root_of_edit_scene(Node* baseNode);
  
  Cesium3DTileset* find_first_tileset(Node* baseNode);

  CesiumGDCreditSystem* find_or_create_credit_system(Node* baseNode, bool deferred);

  CesiumGDConfig* find_or_create_config_node(Node* baseNode);

  Camera3D* find_georef_cam(Node* rootNode);
  
  void update_camera_tilesets(Camera3D* camera);
  
  Array find_all_tilesets(Node* baseNode);
  
  template<class T>
  inline T* find_node_in_scene(Node* root) {
      T* casted_root = Object::cast_to<T>(root);
      if (casted_root != nullptr) {
          return casted_root;
      }

      int32_t count = root->get_child_count();
      for (int32_t i = 0; i < count; i++) {
          Node* child = root->get_child(i);

          T* foundChild = Object::cast_to<T>(child);
          if (foundChild != nullptr) {
              return foundChild;
          }
          
          foundChild = find_node_in_scene<T>(child);
          if (foundChild != nullptr) {
              return foundChild;
          }
      }
      return nullptr;
  }
}
