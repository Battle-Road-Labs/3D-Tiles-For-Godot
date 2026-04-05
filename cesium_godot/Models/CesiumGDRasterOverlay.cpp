#include "CesiumGDRasterOverlay.h"
#include <CesiumRasterOverlays/IonRasterOverlay.h>
#include <CesiumRasterOverlays/TileMapServiceRasterOverlay.h>
#include "CesiumGDTileset.h"
#include "CesiumGDConfig.h"

int64_t CesiumIonRasterOverlay::get_asset_id() const
{
	return this->m_assetId;
}

void CesiumIonRasterOverlay::set_asset_id(int64_t id)
{
	this->m_assetId = id;
}

void CesiumIonRasterOverlay::set_material_key(const String& key)
{
	this->m_materialKey = key;
}

const String& CesiumIonRasterOverlay::get_material_key() const
{
	return this->m_materialKey;
}

void CesiumIonRasterOverlay::set_url(const String& url)
{
	this->m_url = url;
}

const String& CesiumIonRasterOverlay::get_url() const
{
	return this->m_url;
}

int CesiumIonRasterOverlay::get_data_source() const
{
	return static_cast<int>(this->m_selectedDataSource);
}

void CesiumIonRasterOverlay::set_data_source(int data_source)
{
	this->m_selectedDataSource = static_cast<CesiumDataSource>(data_source);
	this->notify_property_list_changed();
}

Error CesiumIonRasterOverlay::add_to_tileset(Cesium3DTileset* tilesetInstance)
{
	if (tilesetInstance == nullptr) return Error::ERR_INVALID_PARAMETER;
	if (this->m_selectedDataSource == CesiumDataSource::FromCesiumIon) {
		if (this->m_assetId <= 0) return Error::ERR_CANT_ACQUIRE_RESOURCE;
	} else {
		if (this->m_url.is_empty()) return Error::ERR_CANT_ACQUIRE_RESOURCE;
	}

	//Overlay already added
	if (this->m_overlayInstance != nullptr) return Error::OK;

	this->create_and_add_overlay(tilesetInstance);
	return Error::OK;
}

void CesiumIonRasterOverlay::remove_from_tileset(Cesium3DTileset* tilesetInstance)
{

}

CesiumUtility::IntrusivePointer<CesiumRasterOverlays::RasterOverlay> CesiumIonRasterOverlay::get_overlay_instance()
{
	return this->m_overlayInstance;
}

void CesiumIonRasterOverlay::create_and_add_overlay(Cesium3DTileset* tilesetInstance)
{
	if (this->m_selectedDataSource == CesiumDataSource::FromCesiumIon) {
		const String& ionAccessToken = CesiumGDConfig::get_singleton(this)->get_access_token();
		this->m_overlayInstance = new CesiumRasterOverlays::IonRasterOverlay(
			this->m_materialKey.utf8().get_data(),
			this->m_assetId,
			ionAccessToken.utf8().get_data(),
			{}
		);
	}
	else {
		CesiumRasterOverlays::TileMapServiceRasterOverlayOptions options{};
		
		this->m_overlayInstance = new CesiumRasterOverlays::TileMapServiceRasterOverlay(
			this->m_materialKey.utf8().get_data(),
			this->m_url.utf8().get_data(),
			std::vector<CesiumAsync::IAssetAccessor::THeader>{},
			options
		);
	}
	tilesetInstance->add_overlay(this);
}

void CesiumIonRasterOverlay::_bind_methods()
{

	ClassDB::bind_method(D_METHOD("set_material_key", "key"), &CesiumIonRasterOverlay::set_material_key);
	ClassDB::bind_method(D_METHOD("get_material_key"), &CesiumIonRasterOverlay::get_material_key);
	ADD_PROPERTY(PropertyInfo(Variant::STRING, "key"), "set_material_key", "get_material_key");


	ClassDB::bind_method(D_METHOD("set_asset_id", "id"), &CesiumIonRasterOverlay::set_asset_id);
	ClassDB::bind_method(D_METHOD("get_asset_id"), &CesiumIonRasterOverlay::get_asset_id);
	ADD_PROPERTY(PropertyInfo(Variant::INT, "asset_id"), "set_asset_id", "get_asset_id");

	ClassDB::bind_method(D_METHOD("get_data_source"), &CesiumIonRasterOverlay::get_data_source);
	ClassDB::bind_method(D_METHOD("set_data_source", "data_source"), &CesiumIonRasterOverlay::set_data_source);
	ADD_PROPERTY(PropertyInfo(Variant::INT, "data_source", PROPERTY_HINT_ENUM, "From Cesium Ion,From Url"), "set_data_source", "get_data_source");
	
	ClassDB::bind_method(D_METHOD("set_url", "url"), &CesiumIonRasterOverlay::set_url);
	ClassDB::bind_method(D_METHOD("get_url"), &CesiumIonRasterOverlay::get_url);
	ADD_PROPERTY(PropertyInfo(Variant::STRING, "url"), "set_url", "get_url");
	
	ClassDB::bind_integer_constant(get_class_static(), "CesiumDataSource", "FromCesiumIon", static_cast<int32_t>(CesiumDataSource::FromCesiumIon));
	ClassDB::bind_integer_constant(get_class_static(), "CesiumDataSource", "FromUrl", static_cast<int32_t>(CesiumDataSource::FromUrl));
}

void CesiumIonRasterOverlay::_get_property_list(List<PropertyInfo>* properties) const
{
	#if defined(CESIUM_GD_MODULE)
	for (int32_t i = 0; i < properties->size(); i++) {
		PropertyInfo& propertyRef = properties->get(i);
	#elif defined(CESIUM_GD_EXT)
	for (auto it = properties->begin(); it != properties->end(); ++it) {
		PropertyInfo& propertyRef = *it;
	#endif
		if (propertyRef.name == StringName("url")) {
			propertyRef.usage = this->m_selectedDataSource == CesiumDataSource::FromCesiumIon ? PROPERTY_USAGE_READ_ONLY : PROPERTY_USAGE_DEFAULT;
		}
		if (propertyRef.name == StringName("asset_id")) {
			propertyRef.usage = this->m_selectedDataSource == CesiumDataSource::FromCesiumIon ? PROPERTY_USAGE_DEFAULT : PROPERTY_USAGE_READ_ONLY;
		}
	}
}

bool CesiumIonRasterOverlay::_set(const StringName& p_name, const Variant& p_property)
{
	if (p_name == StringName("url")) {
		this->set_url(p_property);
		return true;
	}
	if (p_name == StringName("asset_id")) {
		this->set_asset_id(p_property);
		return true;
	}
	return false;
}

bool CesiumIonRasterOverlay::_get(const StringName& p_name, Variant& r_property) const
{
	if (p_name == StringName("url")) {
		r_property = this->get_url();
		return true;
	}
	if (p_name == StringName("asset_id")) {
		r_property = this->get_asset_id();
		return true;
	}

	return false;
}
