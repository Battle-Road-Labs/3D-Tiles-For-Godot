#pragma once

#if defined(CESIUM_GD_EXT)
#include <godot_cpp/classes/node3d.hpp>
using namespace godot;
#elif defined(CESIUM_GD_MODULE)

#endif

class CesiumGlobe;


namespace Godot3DTiles::AssetManipulation {
  
  CesiumGlobe* find_or_create_globe(Node3D* baseNode);

  Node3D* get_root_of_edit_scene(Node3D* baseNode);
  
  
}
