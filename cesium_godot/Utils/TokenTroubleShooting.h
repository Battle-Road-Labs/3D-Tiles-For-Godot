#ifndef TOKEN_TROBLESHOOTING_H
#define TOKEN_TROBLESHOOTING_H

#include "Models/CesiumGDConfig.h"
#include "Utils/CurlHttpClient.h"
#include "godot_cpp/variant/variant.hpp"
#if defined(CESIUM_GD_EXT)
#include "godot_cpp/classes/node.hpp"
using namespace godot;
#endif


class TokenTroubleshooting : public Node {
  GDCLASS(TokenTroubleshooting, Node)
  public:
    void is_valid_token(const String& token, const Ref<CesiumGDConfig>& config);

    virtual void on_token_validity_check(const String& token, bool isValid, const Variant& data);
    
  protected:
    static void _bind_methods();

  private:
    CurlHttpClient<1> m_httpClient{};
    
};

#endif
