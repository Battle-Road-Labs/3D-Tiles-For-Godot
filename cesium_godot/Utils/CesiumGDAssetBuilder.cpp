
#include "CesiumGDAssetBuilder.h"
#include "Models/CesiumGlobe.h"
#include "Utils/AssetManipulation.h"


void CesiumGDAssetBuilder::instantiate_tileset(int32_t tilesetType) {
	//Create a new Globe
	Godot3DTiles::AssetManipulation::instantiate_tileset(this, tilesetType);	
}
	

void CesiumGDAssetBuilder::instantiate_dynamic_cam() {
  Godot3DTiles::AssetManipulation::instantiate_dynamic_cam(this);
}

Variant CesiumGDAssetBuilder::get_georeference_camera_script() const {
	return this->m_georeferenceCameraScript;
}

void CesiumGDAssetBuilder::set_georeference_camera_script(Variant cameraScript) {
	this->m_georeferenceCameraScript = cameraScript;
}

CesiumGlobe* CesiumGDAssetBuilder::find_or_create_globe() {
	return Godot3DTiles::AssetManipulation::find_or_create_globe(this);
}

void CesiumGDAssetBuilder::_bind_methods() {
	ClassDB::bind_method(D_METHOD("instantiate_dynamic_cam"), &CesiumGDAssetBuilder::instantiate_dynamic_cam);
	ClassDB::bind_method(D_METHOD("instantiate_tileset", "type"), &CesiumGDAssetBuilder::instantiate_tileset);
	ClassDB::bind_method(D_METHOD("find_or_create_globe"), &CesiumGDAssetBuilder::find_or_create_globe);	

	ClassDB::bind_method(D_METHOD("get_georeference_camera_script"), &CesiumGDAssetBuilder::get_georeference_camera_script);
	ClassDB::bind_method(D_METHOD("set_georeference_camera_script", "script"), &CesiumGDAssetBuilder::set_georeference_camera_script);
	
	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "georef_camera_script"), "set_georeference_camera_script", "get_georeference_camera_script");
}
