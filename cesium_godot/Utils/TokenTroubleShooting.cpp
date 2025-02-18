#include "TokenTroubleShooting.h"
#include "CesiumIonClient/Connection.h"
#include "Implementations/NetworkAssetAccessor.h"
#include <memory>

bool TokenTroubleshooting::is_valid_token(const String& token, const Ref<CesiumGDConfig>& config) {
  //Get the connection
  auto simpleAccessor = std::make_shared<NetworkAssetAccessor>();
  auto connection = std::make_shared<CesiumIonClient::Connection>(
    {},
    simpleAccessor,
    token.utf8().get_data(),
    config->get_api_url().utf8().get_data()
  );
  return false; 
}
