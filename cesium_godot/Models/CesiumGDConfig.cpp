#include "CesiumGDConfig.h"
#include "Utils/AssetManipulation.h"
#include "error_names.hpp"
#if defined(CESIUM_GD_EXT)
#include "godot_cpp/classes/dir_access.hpp"
#include "godot_cpp/classes/file_access.hpp"
#include "godot_cpp/classes/global_constants.hpp"
#include "godot_cpp/classes/project_settings.hpp"
#include "godot_cpp/core/class_db.hpp"
#include "godot_cpp/core/error_macros.hpp"
#else
#include "core/io/dir_access.h"
#include "core/io/file_access.h"
#endif
#include "missing_functions.hpp"


constexpr const char* CACHE_PATH = "user://cache";

#define CONFIG_FILE_PATH String(CACHE_PATH) + "/ion_session.dat"

void CesiumGDConfig::set_access_token(const String& accessToken)
{
	// Write to cache
	String errMsg("Could not write access token to cache path, error: ");
	Error err = this->create_cache_session_file();
	if (err != Error::OK) {
		return;
	}
	
	Ref<FileAccess> file = open_file_access_with_err(CONFIG_FILE_PATH, FileAccess::ModeFlags::WRITE, &err);
	if (err != Error::OK) {
		ERR_PRINT(errMsg + REFLECT_ERR_NAME(err));
	}
	
	// Keep both in memory and disk for next access when entering the tree
	file->store_string(accessToken);
	this->m_accessToken = accessToken;
	//Flush resources
	file->close();
}

const String& CesiumGDConfig::get_access_token() const
{
	return this->m_accessToken;
}

void CesiumGDConfig::clear_session() {
	// We could delete the cache if we need to, but we might be able to get away with just setting it once + no longer 	
}

CesiumGDConfig* CesiumGDConfig::get_singleton(Node* baseNode) {
	if (s_instance != nullptr) {
		return s_instance;
	}
	
	s_instance = Godot3DTiles::AssetManipulation::find_or_create_config_node(baseNode);

	
	// Write to cache
	// Try to read the token data
	Error err = s_instance->create_cache_session_file();
	if (err != Error::OK){
		return s_instance;
	}
	
	Ref<FileAccess> file = open_file_access_with_err(CONFIG_FILE_PATH, FileAccess::ModeFlags::READ, &err);
	
	if (err != Error::OK) {
		ERR_PRINT("Error opening cached session file:" + String(REFLECT_ERR_NAME(err)) + ", please try to connect to Cesium ION and reopen the project! ");
	}
	    
	String token = file->get_as_text();
	file->close();
	
	s_instance->set_access_token(token);
	
	if (s_instance == nullptr) {
		ERR_PRINT("Failed to find / Create a CesiumConfig node, try adding it manually!");
	}
	
	return s_instance;
}


Error CesiumGDConfig::create_cache_session_file() {
	// Create dir recursive	
	String errMsg("Could not write access token to cache path, error: ");
	Ref<DirAccess> userAccess = DirAccess::open("user://");
	Error err = userAccess->make_dir_recursive(CACHE_PATH);
	if (err != Error::OK) {
		ERR_PRINT(errMsg + REFLECT_ERR_NAME(err));
		return err;
	}
	// Now check if the file exists
	if (userAccess->file_exists(CONFIG_FILE_PATH)) {
		return Error::OK;
	}

	// Will create the empty file, do not use this when the file already exists since it will truncate it
	Ref<FileAccess> file = open_file_access_with_err(CONFIG_FILE_PATH, FileAccess::ModeFlags::WRITE_READ, &err);
	file->close();
	return err;
}

void CesiumGDConfig::_enter_tree() {
}

void CesiumGDConfig::_bind_methods()
{

	ClassDB::bind_method(D_METHOD("get_access_token"), &CesiumGDConfig::get_access_token);
	ClassDB::bind_method(D_METHOD("set_access_token", "accessToken"), &CesiumGDConfig::set_access_token);
	ADD_PROPERTY(PropertyInfo(Variant::STRING, "accessToken"), "set_access_token", "get_access_token");
	
	ClassDB::bind_static_method("CesiumGDConfig", D_METHOD("get_singleton", "baseNode"), CesiumGDConfig::get_singleton);
	
}
