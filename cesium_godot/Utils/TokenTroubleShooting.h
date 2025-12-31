#ifndef TOKEN_TROBLESHOOTING_H
#define TOKEN_TROBLESHOOTING_H

#ifdef __APPLE__
#include "Utils/GodotHttpClient.h"
#else
#include "Utils/CurlHttpClient.h"
#endif
#include "godot_cpp/variant/variant.hpp"
#include <cstdint>
#include <unordered_map>
#if defined(CESIUM_GD_EXT)
#include "godot_cpp/classes/node.hpp"
using namespace godot;
#endif


class TokenTroubleshooting : public Node {
  GDCLASS(TokenTroubleshooting, Node)
  public:
    void is_valid_token(const String& token);

    void on_token_validity_check(const String& token, bool isValid, const std::unordered_map<std::string, int32_t>& data);
    
    void set_data(const Variant &data);

    int32_t get_asset_id_by_name(const String &name) const {
      return this->m_lastAssetLists.at(name.utf8().get_data());
    }

    void _exit_tree() override;
    
  protected:
    static void _bind_methods();

  private:
#ifdef __APPLE__
    GodotHttpClient<1> m_httpClient{};
#else
    CurlHttpClient<1> m_httpClient{};
#endif
    Variant m_tokenData;
    std::unordered_map<std::string, int32_t> m_lastAssetLists;   
};

#endif
