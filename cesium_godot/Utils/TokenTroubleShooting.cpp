#include "TokenTroubleShooting.h"
#include "error_names.hpp"
#if defined(CESIUM_GD_EXT)
#include "godot_cpp/classes/http_client.hpp"
#include "godot_cpp/classes/json.hpp"
#include "godot_cpp/classes/os.hpp"
#include "godot_cpp/core/class_db.hpp"
#include "godot_cpp/core/error_macros.hpp"
#include "godot_cpp/variant/dictionary.hpp"
#include "godot_cpp/variant/packed_byte_array.hpp"
#include "godot_cpp/variant/packed_string_array.hpp"
#include "godot_cpp/variant/string.hpp"
#else
#include "core/io/json.h"
#include "core/io/http_client.h"
#endif
#include <cstdint>
#include <string_view>
#include <unordered_map>

constexpr std::string_view ASSET_LIST_URL =
    "https://api.cesium.com/v1/assets?access_token=";


void TokenTroubleshooting::is_valid_token(const String& token) {
  m_httpClient.init_client(1);
  std::string request = ASSET_LIST_URL.data();
  request.append(token.utf8().get_data());
  m_httpClient.send_get(
      request.c_str(),
      [this, token](int32_t status, const PackedByteArray &body) { 
        // Get either a list of the available assets, or the error message
        Ref<JSON> jsonObj = memnew(JSON);
        std::string tmpStr(reinterpret_cast<const char*>(body.ptr()), body.size());
        String bodyStr = tmpStr.c_str();
        
        Error parseErr = jsonObj->parse(bodyStr);
        if (parseErr != Error::OK ||
            jsonObj->get_data().get_type() != Variant::DICTIONARY) {
          String msg =
              String("There was an error getting the body of the request: ") +
              REFLECT_ERR_NAME(parseErr);
          ERR_PRINT(msg);
          on_token_validity_check(token, false,{ {msg.utf8().get_data(), 0}});
          return;
        }

        if (status >= HTTPClient::ResponseCode::RESPONSE_BAD_REQUEST) {
          String msg = jsonObj->get_data().get("message");
          on_token_validity_check(token, false,{ {msg.utf8().get_data(), 0} });
          return;
        }

        // Build the list of assets
        std::unordered_map<std::string, int32_t> assetDict{};
        // Get the items
        bool hasItems;
        Array items = jsonObj->get_data().get("items", &hasItems);

        for (int32_t i = 0; i < items.size(); i++) {
          const Variant &currItem = items[i];
          if (currItem.get_type() != Variant::Type::DICTIONARY)
            continue;
          const String &assetName = currItem.get("name");
          int32_t assetId = currItem.get("id");
          assetDict.insert_or_assign(assetName.utf8().get_data(), assetId);
        }
        on_token_validity_check(token, hasItems, assetDict);
      },
      {});
}


void TokenTroubleshooting::set_data(const Variant &p_data) {
  this->m_tokenData = p_data;
  Variant asset_list_group = this->m_tokenData.get("asset_list_group");
  asset_list_group.set("visible", false);
}


void TokenTroubleshooting::on_token_validity_check(const String& token, bool isValid, const std::unordered_map<std::string, int32_t>& p_data) {
  if (!isValid) {
    OS::get_singleton()->alert("Token is not valid, try signing into Cesium ION");
    return;
  }
  // Show the asset list group.
  Variant asset_list_group = m_tokenData.get("asset_list_group");
  asset_list_group.set("visible", true);
  // Add each entry from data to token_data.asset_list_items.
  Variant asset_list_items = m_tokenData.get("asset_list_items");
  this->m_lastAssetLists = p_data;
  Variant asset_id_text = m_tokenData.get("asset_id_text");
  if (asset_list_items.get_type() == Variant::OBJECT) {
      Object *list_items = asset_list_items;
      for (const auto& kv : p_data) {
        list_items->call("add_item", kv.first.c_str());
      }
  }
}

void TokenTroubleshooting::_exit_tree() {
  
  Variant asset_list_group = m_tokenData.get("asset_list_group");
  if (asset_list_group.get_type() == Variant::OBJECT) {
    Object *group = asset_list_group;
    group->set("visible", false);
  }
}

void TokenTroubleshooting::_bind_methods() {
  ClassDB::bind_method(
      D_METHOD("is_valid_token", "token"),
      &TokenTroubleshooting::is_valid_token
  );

  ClassDB::bind_method(D_METHOD("set_data", "data"), &TokenTroubleshooting::set_data);
  ClassDB::bind_method(D_METHOD("get_asset_id_by_name"), &TokenTroubleshooting::get_asset_id_by_name);
  
}
