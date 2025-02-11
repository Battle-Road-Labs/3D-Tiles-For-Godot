#include "CesiumGDConfig.h"

void CesiumGDConfig::set_access_token(const String& accessToken)
{
	this->m_accessToken = accessToken;
}

const String& CesiumGDConfig::get_access_token() const
{
	return this->m_accessToken;
}

void CesiumGDConfig::set_server_url(const String& serverUrl)
{
	this->m_serverUrl = serverUrl;
}

const String& CesiumGDConfig::get_server_url() const
{
	return this->m_serverUrl;
}

void CesiumGDConfig::set_api_url(const String& apiUrl)
{
	this->m_apiUrl = apiUrl;
}

const String& CesiumGDConfig::get_api_url() const
{
	return this->m_apiUrl;
}

void CesiumGDConfig::set_application_id(int32_t applicationId)
{
	this->m_applicationId = applicationId;
}

int32_t CesiumGDConfig::get_application_id() const
{
	return this->m_applicationId;
}

void CesiumGDConfig::_bind_methods()
{
	ClassDB::bind_method(D_METHOD("get_server_url"), &CesiumGDConfig::get_server_url);
	ClassDB::bind_method(D_METHOD("set_server_url", "serverUrl"), &CesiumGDConfig::set_server_url);
	ADD_PROPERTY(PropertyInfo(Variant::STRING, "serverUrl"), "set_server_url", "get_server_url");

	ClassDB::bind_method(D_METHOD("get_access_token"), &CesiumGDConfig::get_access_token);
	ClassDB::bind_method(D_METHOD("set_access_token", "accessToken"), &CesiumGDConfig::set_access_token);
	ADD_PROPERTY(PropertyInfo(Variant::STRING, "accessToken"), "set_access_token", "get_access_token");

	ClassDB::bind_method(D_METHOD("get_api_url"), &CesiumGDConfig::get_api_url);
	ClassDB::bind_method(D_METHOD("set_api_url", "apiUrl"), &CesiumGDConfig::set_api_url);
	ADD_PROPERTY(PropertyInfo(Variant::STRING, "apiUrl"), "set_api_url", "get_api_url");

	ClassDB::bind_method(D_METHOD("get_application_id"), &CesiumGDConfig::get_application_id);
	ClassDB::bind_method(D_METHOD("set_application_id", "applicationId"), &CesiumGDConfig::set_application_id);
	ADD_PROPERTY(PropertyInfo(Variant::INT, "applicationId"), "set_application_id", "get_application_id");

}
