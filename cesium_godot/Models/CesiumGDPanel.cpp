#include "CesiumGDPanel.h"
#include "../Utils/NetworkUtils.h"

void CesiumGDPanel::open_learn_page()
{
	NetworkUtils::SystemOpenURL("https://cesium.com/learn/");
}

void CesiumGDPanel::open_help_page()
{
	NetworkUtils::SystemOpenURL("https://community.cesium.com/");
}

void CesiumGDPanel::_bind_methods()
{
	ClassDB::bind_static_method("CesiumGDPanel", D_METHOD("open_learn_page"), &CesiumGDPanel::open_learn_page);
	ClassDB::bind_static_method("CesiumGDPanel", D_METHOD("open_help_page"), &CesiumGDPanel::open_help_page);
}
