#pragma once

#include "Models/CesiumGDCreditSystem.h"
#include "Models/CesiumGDTileset.h"
#include <algorithm>
#if defined(CESIUM_GD_EXT)
#include <godot_cpp/classes/node3d.hpp>
using namespace godot;
#elif defined(CESIUM_GD_MODULE)

#endif

class CesiumGlobe;


namespace Godot3DTiles::AssetManipulation {

  enum class TilesetType : int32_t {
    Blank,
    OsmBuildings,
    GooglePhotorealistic,
    BingMapsAerialWithLabels,
    BingMapsRoads
  };  


  void instantiate_tileset(Node3D* baseNode, int32_t tilesetType);
  
  void instantiate_dynamic_cam(Node3D* baseNode);
  
  CesiumGlobe* find_or_create_globe(Node3D* baseNode);

  Node3D* get_root_of_edit_scene(Node3D* baseNode);
  
  CesiumGDTileset* find_first_tileset(Node3D* baseNode);

  CesiumGDCreditSystem* find_or_create_credit_system(Node3D* baseNode, bool deferred);
  
  
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
