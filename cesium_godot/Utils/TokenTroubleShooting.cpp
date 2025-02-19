#include "TokenTroubleShooting.h"
#include "error_names.hpp"
#include "godot_cpp/classes/http_client.hpp"
#include "godot_cpp/classes/json.hpp"
#include "godot_cpp/core/class_db.hpp"
#include "godot_cpp/core/error_macros.hpp"
#include "godot_cpp/variant/string.hpp"
#include <cstdint>
#include <string_view>

constexpr std::string_view ASSET_LIST_URL =
    "https://api.cesium.com/v1/assets?access_token=";

void TokenTroubleshooting::is_valid_token(const String &token,
                                          const Ref<CesiumGDConfig> &config,
                                          const Callable &callback) {
  m_httpClient.init_client(1);
  std::string request = ASSET_LIST_URL.data();
  request.append(token.utf8().get_data());
  m_httpClient.send_get(
      request.c_str(),
      [callback](int32_t status, const Vector<uint8_t> &body) {
        // Get either a list of the available assets, or the error message
        Ref<JSON> jsonObj = memnew(JSON);
        const char *strPtr = reinterpret_cast<const char *>(body.ptr());
        String bodyStr = strPtr;
        WARN_PRINT(bodyStr);
        Error parseErr = jsonObj->parse(bodyStr);
        if (parseErr != Error::OK ||
            jsonObj->get_data().get_type() != Variant::DICTIONARY) {
          String msg =
              String("There was an error getting the body of the request: ") +
              REFLECT_ERR_NAME(parseErr);
          ERR_PRINT(msg);
          callback.call(false, msg);
          return;
        }

        if (status >= HTTPClient::ResponseCode::RESPONSE_BAD_REQUEST) {
          String msg = jsonObj->get_data().get("message");
          callback.call(false, msg);
          return;
        }

        // Build the list of assets
        Vector<String> assetList;

        // Get the items
        bool hasItems;
        Array items = jsonObj->get_data().get("items", &hasItems);

        for (int32_t i = 0; i < items.size(); i++) {
          const Variant &currItem = items[i];
          if (currItem.get_type() != Variant::Type::DICTIONARY)
            continue;
          const String &assetName = currItem.get("name");
          int32_t assetId = currItem.get("id");
          assetList.push_back(assetName + String("; Asset id: ") + assetId);
        }

        callback.call(hasItems, assetList);
      },
      {});
}

void TokenTroubleshooting::_bind_methods() {
  ClassDB::bind_static_method(
      "TokenTroubleshooting",
      D_METHOD("is_valid_token", "token", "config", "callback"),
      &TokenTroubleshooting::is_valid_token
  );

  
  
}
