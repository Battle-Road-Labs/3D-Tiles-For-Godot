#ifndef TOKEN_TROBLESHOOTING_H
#define TOKEN_TROBLESHOOTING_H

#include "Models/CesiumGDConfig.h"
#include "Utils/CurlHttpClient.h"
#include "godot_cpp/variant/callable.hpp"
#if defined(CESIUM_GD_EXT)
#include "godot_cpp/classes/node.hpp"
using namespace godot;
#endif


class TokenTroubleshooting : public Node {
  GDCLASS(TokenTroubleshooting, Node)
  public:
    static void is_valid_token(const String& token, const Ref<CesiumGDConfig>& config, const Callable& callback);

  protected:
    static void _bind_methods();

  private:
    static inline CurlHttpClient<1> m_httpClient{};
    
};

#endif
