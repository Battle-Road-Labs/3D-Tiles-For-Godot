#include "CesiumGDCreditSystem.h"

#include "CesiumUtility/CreditSystem.h"
#include "Implementations/DocumentContainer.h"
#include "Utils/AssetManipulation.h"
#if defined(CESIUM_GD_EXT)
#include "godot_cpp/classes/control.hpp"
#include "godot_cpp/classes/node.hpp"
#include "godot_cpp/classes/object.hpp"
#include "godot_cpp/classes/scene_tree.hpp"
#include "godot_cpp/core/error_macros.hpp"
#include "godot_cpp/core/memory.hpp"
#include "godot_cpp/variant/vector2.hpp"
#else
#endif
#include "missing_functions.hpp"

CesiumGDCreditSystem* CesiumGDCreditSystem::get_singleton(Node3D* baseNode) {
  if (s_instance != nullptr) {
    return s_instance;
  }
  s_instance = Godot3DTiles::AssetManipulation::find_or_create_credit_system(baseNode, false);
  if (s_instance == nullptr) {
    ERR_PRINT("Could not find Credit System Node in the CesiumGeoreference, try adding it manually");
    return nullptr;
  }
  return s_instance;
}

void CesiumGDCreditSystem::_process(real_t p_delta) {
  this->update_credits();
}


void CesiumGDCreditSystem::update_credits() {
  if (is_editor_mode()) return;
  std::string finalHtml;
  if (!this->m_creditSystems.empty()) {
    finalHtml = "";
  }
  
  for (const auto& creditSystem : this->m_creditSystems) {
    const auto& creditSnapshot = creditSystem->getSnapshot();
    for (const CesiumUtility::Credit& credit : creditSnapshot.currentCredits) {
      const std::string& html = creditSystem->getHtml(credit) + "\n";
      finalHtml += html.c_str();
    }
  }
  if (finalHtml == this->m_lastHtml) {
    return;
  }

  this->m_lastHtml = finalHtml;
  this->m_rect->set_html_stl(finalHtml);
}

  void CesiumGDCreditSystem::add_credit_system(std::shared_ptr<CesiumUtility::CreditSystem> creditSystem) {
  this->m_creditSystems.emplace_back(creditSystem);
}

void CesiumGDCreditSystem::_enter_tree() {
  if (!is_editor_mode()){
    this->set_process(true);
  }   
  // Create the HTML rect and set its html to something to test it
  s_instance = this;
  
  this->m_creditSystems.reserve(3);
  if (this->get_child_count() > 0) {
      this->m_rect = Object::cast_to<DocumentContainer>(this->get_child(0));
      if (this->m_rect != nullptr) {
        return;
      }
  }

  
  this->set_anchors_preset(Control::LayoutPreset::PRESET_BOTTOM_LEFT);
  this->set_offset(Side::SIDE_TOP, -150.0f);
  this->set_offset(Side::SIDE_LEFT, 0.0f);
  this->set_offset(Side::SIDE_BOTTOM, 0.0f);
  this->m_rect = memnew(DocumentContainer);
  this->m_rect->set_size(Vector2(500, 100));
  this->add_child(this->m_rect, false, INTERNAL_MODE_FRONT);
  this->m_rect->set_owner(this->get_parent());
}

void CesiumGDCreditSystem::_bind_methods() {
  
}
