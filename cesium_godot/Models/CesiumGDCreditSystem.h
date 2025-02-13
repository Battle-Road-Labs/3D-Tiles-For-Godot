#ifndef CESIUM_GD_CREDIT_SYSTEM
#define CESIUM_GD_CREDIT_SYSTEM

#if defined(CESIUM_GD_EXT)
#include "godot_cpp/classes/node3d.hpp"
using namespace godot;
#elif defined(CESIUM_GD_MODULE)
#endif

#include <memory.h>

namespace CesiumUtility {
  class CreditSystem;
}

class CesiumGDCreditSystem : Node3D {
  GDCLASS(CesiumGDCreditSystem, Node3D)
  public:
    static CesiumGDCreditSystem* get_singleton();

    CesiumGDCreditSystem();
    
  private:
     std::shared_ptr<CesiumUtility::CreditSystem> m_creditSystem;
     static inline CesiumGDCreditSystem*  s_instance = nullptr;
};

#endif
