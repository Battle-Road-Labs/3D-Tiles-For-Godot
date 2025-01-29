#include "CesiumGDRasterOverlay.h"
#include "../Utils/NetworkUtils.h"
#include <CesiumRasterOverlays/IonRasterOverlay.h>
#include "CesiumGDTileset.h"
#include "CesiumGDConfig.h"

int64_t CesiumGDRasterOverlay::get_asset_id() const
{
	return this->m_assetId;
}

void CesiumGDRasterOverlay::set_asset_id(int64_t id)
{
	this->m_assetId = id;
}

void CesiumGDRasterOverlay::set_ion_access_token(const String& token)
{
	this->m_cesiumIonToken = token;
}

const String& CesiumGDRasterOverlay::get_ion_access_token() const
{
	return this->m_cesiumIonToken;
}

void CesiumGDRasterOverlay::set_material_key(const String& key)
{
	this->m_materialKey = key;
}

const String& CesiumGDRasterOverlay::get_material_key() const
{
	return this->m_materialKey;
}

Error CesiumGDRasterOverlay::add_to_tileset(CesiumGDTileset* tilesetInstance)
{
	if (tilesetInstance == nullptr) return Error::ERR_INVALID_PARAMETER;
	this->m_configInstance = tilesetInstance->get_cesium_config();
	if (this->m_assetId <= 0) return Error::ERR_CANT_ACQUIRE_RESOURCE;

	//Overlay already added
	if (this->m_overlayInstance != nullptr) return Error::OK;

	if (this->m_cesiumIonToken.is_empty()) {
		//TODO: Get the token from the default OAuth login
	}
	this->create_and_add_overlay(tilesetInstance);
	return Error::OK;
}

void CesiumGDRasterOverlay::remove_from_tileset(CesiumGDTileset* tilesetInstance)
{

}

CesiumUtility::IntrusivePointer<CesiumRasterOverlays::IonRasterOverlay> CesiumGDRasterOverlay::get_overlay_instance()
{
	return this->m_overlayInstance;
}

void CesiumGDRasterOverlay::create_and_add_overlay(CesiumGDTileset* tilesetInstance)
{
	this->m_overlayInstance = new CesiumRasterOverlays::IonRasterOverlay(
		this->m_materialKey.utf8().get_data(),
		this->m_assetId,
		this->m_cesiumIonToken.utf8().get_data(),
		{},
		this->m_configInstance->get_api_url().utf8().get_data()
	);
	tilesetInstance->add_overlay(this);
}

void CesiumGDRasterOverlay::_bind_methods()
{

	ClassDB::bind_method(D_METHOD("set_material_key", "key"), &CesiumGDRasterOverlay::set_material_key);
	ClassDB::bind_method(D_METHOD("get_material_key"), &CesiumGDRasterOverlay::get_material_key);
	ADD_PROPERTY(PropertyInfo(Variant::STRING, "key"), "set_material_key", "get_material_key");


	ClassDB::bind_method(D_METHOD("set_asset_id", "id"), &CesiumGDRasterOverlay::set_asset_id);
	ClassDB::bind_method(D_METHOD("get_asset_id"), &CesiumGDRasterOverlay::get_asset_id);
	ADD_PROPERTY(PropertyInfo(Variant::INT, "asset_id"), "set_asset_id", "get_asset_id");

	ClassDB::bind_method(D_METHOD("set_ion_access_token", "token"), &CesiumGDRasterOverlay::set_ion_access_token);
	ClassDB::bind_method(D_METHOD("get_ion_access_token"), &CesiumGDRasterOverlay::get_ion_access_token);
	ADD_PROPERTY(PropertyInfo(Variant::STRING, "ion_access_token"), "set_ion_access_token", "get_ion_access_token");

}
