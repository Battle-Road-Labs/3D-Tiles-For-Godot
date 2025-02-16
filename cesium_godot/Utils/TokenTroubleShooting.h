#ifndef TOKEN_TROBLESHOOTING_H
#define TOKEN_TROBLESHOOTING_H

#include "Models/CesiumGDConfig.h"
#if defined(CESIUM_GD_EXT)
#include "godot_cpp/classes/node.hpp"
using namespace godot;
#endif


class TokenTroubleshooting : public Node {
  GDCLASS(TokenTroubleshooting, Node)
  public:
    static bool is_valid_token(const String& token, const Ref<CesiumGDConfig>& config);

  protected:
    static void _bind_methods();
    
};

#endif
