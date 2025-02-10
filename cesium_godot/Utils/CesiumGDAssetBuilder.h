
#ifndef CESIUM_GD_ASSET_BUILDER
#define CESIUM_GD_ASSET_BUILDER

#include <cstdint>
#if defined(CESIUM_GD_EXT)
#include <godot_cpp/classes/node3d.hpp>
using namespace godot;
#elif defined(CESIUM_GD_MODULE)

#endif

/// @brief Simple front-facing UI for the AssetManipulation namespace
class CesiumGDAssetBuilder : public Node3D {
  GDCLASS(CesiumGDAssetBuilder, Node3D)
public:
  void instantiate_tileset(int32_t tilesetType);

  void instantiate_dynamic_cam();

  Variant get_georeference_camera_script() const;

  void set_georeference_camera_script(Variant cameraScript);

private:
  Variant m_georeferenceCameraScript;
  Variant m_normalCameraScript;

protected:
  static void _bind_methods();
};

#endif
