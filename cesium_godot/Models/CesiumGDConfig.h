#ifndef CESIUM_GD_CONFIG
#define CESIUM_GD_CONFIG

#if defined(CESIUM_GD_EXT)
#include <godot_cpp/classes/resource.hpp>
using namespace godot;
#elif defined(CESIUM_GD_MODULE)
#include "core/io/resource.h"
#endif

/// @brief Configuration resource for Cesium's server components
class CesiumGDConfig : public Resource {
	GDCLASS(CesiumGDConfig, Resource)
public:
	static constexpr std::string_view DEFAULT_ION_API_URL = "https://api.cesium.com/";
	static constexpr std::string_view DEFAULT_SERVER_URL = "https://ion.cesium.com";

	CesiumGDConfig() = default;

	void set_access_token(const String& accessToken);

	const String& get_access_token() const;

	void set_server_url(const String& serverUrl);

	const String& get_server_url() const;

	void set_api_url(const String& apiUrl);

	const String& get_api_url() const;

	void set_application_id(int32_t applicationId);

	int32_t get_application_id() const;

private:
	String m_accessToken{};
	String m_serverUrl = DEFAULT_SERVER_URL.data();
	String m_apiUrl = DEFAULT_ION_API_URL.data();
	int32_t m_applicationId = 891;
protected:
	static void _bind_methods();
};

#endif // CESIUM_GD_CONFIG
