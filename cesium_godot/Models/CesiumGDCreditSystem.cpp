#include "CesiumGDCreditSystem.h"

#include "CesiumUtility/CreditSystem.h"
#include "Ultralight/String.h"
#include "Utils/AssetManipulation.h"
#include "godot/html_rect/html_rect.hpp"
#include "godot_cpp/classes/control.hpp"
#include "godot_cpp/classes/node.hpp"
#include "godot_cpp/classes/object.hpp"
#include "godot_cpp/classes/scene_tree.hpp"
#include "godot_cpp/core/error_macros.hpp"
#include "godot_cpp/core/memory.hpp"
#include "missing_functions.hpp"

CesiumGDCreditSystem* CesiumGDCreditSystem::get_singleton(Node3D* baseNode) {
  if (s_instance != nullptr) {
    return s_instance;
  }
  s_instance = Godot3DTiles::AssetManipulation::find_or_create_credit_system(baseNode, false);
  printf("Found instance of credit system!\n");
  if (s_instance == nullptr) {
    ERR_PRINT("Could not find Credit System Node in the CesiumGlobe, try adding it manually");
    return nullptr;
  }
  return s_instance;
}

void CesiumGDCreditSystem::_process(double p_delta) {
  this->update_credits();
}


void CesiumGDCreditSystem::update_credits() {
  if (is_editor_mode()) return;
  ultralight::String finalHtml;
  if (!this->m_creditSystems.empty()) {
    finalHtml = "";
    //this->m_rect->set_html("");
  }
  
  for (const auto& creditSystem : this->m_creditSystems) {
    const std::vector<CesiumUtility::Credit>& creditsToShow = creditSystem->getCreditsToShowThisFrame();
    for (const CesiumUtility::Credit& credit : creditsToShow) {
      const std::string& html = creditSystem->getHtml(credit);
      finalHtml += html.c_str();
    }
  }
  if (std::strncmp(this->m_rect->get_html().utf8().data(), finalHtml.utf8().data(), finalHtml.utf8().size()) == 0) {
    return;
  }

  this->m_rect->set_html(finalHtml);
  //printf("%s\n", finalHtml.utf8().get_data());*/
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
      this->m_rect = Object::cast_to<HtmlRect>(this->get_child(0));
      if (this->m_rect != nullptr) {
        return;
      }
  }

  this->set_anchors_preset(Control::LayoutPreset::PRESET_BOTTOM_LEFT);
  this->set_offset(Side::SIDE_TOP, -150.0f);
  this->m_rect = memnew(HtmlRect);
  this->add_child(this->m_rect, false, INTERNAL_MODE_FRONT);
  this->m_rect->set_owner(this->get_parent());
}

void CesiumGDCreditSystem::_bind_methods() {
  
}
