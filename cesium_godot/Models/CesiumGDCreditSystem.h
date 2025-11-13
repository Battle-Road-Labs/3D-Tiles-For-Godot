#ifndef CESIUM_GD_CREDIT_SYSTEM
#define CESIUM_GD_CREDIT_SYSTEM

#include "Implementations/DocumentContainer.h"
#include "executable_node.hpp"
#include <memory>

#if defined(CESIUM_GD_EXT)

#include "godot_cpp/classes/box_container.hpp"
#include "godot_cpp/classes/node3d.hpp"
using namespace godot;

#else
// #include "scene/gui/box_container.h"
#endif

#include <memory.h>

namespace CesiumUtility {
  class CreditSystem;
}



#ifndef _EXE_CONTROL
#define _EXE_CONTROL
MAKE_EXE_CONTROL(ExecutableControl, Control);
#endif

class CesiumGDCreditSystem : public ExecutableControl {
  GDCLASS(CesiumGDCreditSystem, ExecutableControl)
  public:
    static CesiumGDCreditSystem* get_singleton(Node3D* baseNode);

    CesiumGDCreditSystem() = default;
    
    void add_credit_system(std::shared_ptr<CesiumUtility::CreditSystem> creditSystem);
    
    void update_credits();
    
    void _process(real_t p_delta) override;  

    void _enter_tree() override;
    
  private:
    // HtmlRect* m_rect;
    DocumentContainer* m_rect = nullptr;
    std::string m_lastHtml;
    std::vector<std::shared_ptr<CesiumUtility::CreditSystem>> m_creditSystems;
    static inline CesiumGDCreditSystem*  s_instance = nullptr;
     
  protected:
    static void _bind_methods();
};

#endif
