#ifndef CESIUM_GD_CREDIT_SYSTEM
#define CESIUM_GD_CREDIT_SYSTEM

#include "godot/html_rect/html_rect.hpp"
#include "godot_cpp/classes/box_container.hpp"
#include "godot_cpp/classes/node3d.hpp"
#include <memory>
#if defined(CESIUM_GD_EXT)

using namespace godot;
#elif defined(CESIUM_GD_MODULE)
#endif

#include <memory.h>

namespace CesiumUtility {
  class CreditSystem;
}

class CesiumGDCreditSystem : public BoxContainer {
  GDCLASS(CesiumGDCreditSystem, BoxContainer)
  public:
    static CesiumGDCreditSystem* get_singleton(Node3D* baseNode);

    CesiumGDCreditSystem() = default;
    
    void add_credit_system(std::shared_ptr<CesiumUtility::CreditSystem> creditSystem);
    
    void update_credits();
    
    void _process(double p_delta) override;  

    void _enter_tree() override;
    
  private:
    HtmlRect* m_rect;
    std::vector<std::shared_ptr<CesiumUtility::CreditSystem>> m_creditSystems;
    static inline CesiumGDCreditSystem*  s_instance = nullptr;
     
  protected:
    static void _bind_methods();
};

#endif
