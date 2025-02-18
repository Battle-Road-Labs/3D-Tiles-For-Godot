#include "CesiumGDCreditSystem.h"

#include "CesiumUtility/CreditSystem.h"
#include "Utils/AssetManipulation.h"
#include "godot/html_rect/html_rect.hpp"
#include "godot_cpp/classes/control.hpp"
#include "godot_cpp/classes/object.hpp"
#include "godot_cpp/classes/scene_tree.hpp"
#include "godot_cpp/core/error_macros.hpp"
#include "godot_cpp/core/memory.hpp"
#include "godot_cpp/variant/vector2.hpp"
#include "missing_functions.hpp"

CesiumGDCreditSystem* CesiumGDCreditSystem::get_singleton(Node3D* baseNode) {
  if (s_instance != nullptr) {
    return s_instance;
  }
  Node* root = baseNode->get_tree()->get_current_scene();
  int32_t count = root->get_child_count();
  for (int32_t i = 0; i < count; i++) {
    Node* currChild = root->get_child(i);
    s_instance = Object::cast_to<CesiumGDCreditSystem>(currChild);
    if (s_instance != nullptr) {
      return s_instance;
    }
  }
  ERR_PRINT("Could not find Credit System Node in the CesiumGlobe, try adding it manually");
  return nullptr;
}

void CesiumGDCreditSystem::_process(double p_delta) {
  this->update_credits();
}


void CesiumGDCreditSystem::update_credits() {
  if (is_editor_mode()) return;
  String finalHtml = this->m_rect->get_html();
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
  if (this->m_rect->get_html() == finalHtml) {
    printf("Same HTML!\n");
    return;
  }
  printf("Setting html!\n");
  this->m_rect->set_html(finalHtml);
  //printf("%s\n", finalHtml.utf8().get_data());*/
}

  void CesiumGDCreditSystem::add_credit_system(std::shared_ptr<CesiumUtility::CreditSystem> creditSystem) {
  this->m_creditSystems.emplace_back(creditSystem);
}

void CesiumGDCreditSystem::_enter_tree() {
  printf("Enter tree!\n");
  if (!is_editor_mode()){
    printf("Enable processing!\n");
    this->m_rect = Object::cast_to<HtmlRect>(this->get_child(0));
    this->set_process(true);
    return;
  } 
  printf("Disable processing\n");
  this->set_process(false);
  // Create the HTML rect and set its html to something to test it
  Node3D* root = Godot3DTiles::AssetManipulation::get_root_of_edit_scene(this);
  s_instance = this;
  printf("Enter tree, added to singleton\n");
  this->m_creditSystems.reserve(3);
  if (this->get_child_count() > 0) {
      this->m_rect = Object::cast_to<HtmlRect>(this->get_child(0));
      if (this->m_rect != nullptr) {
        return;
      }
  }
  this->m_rect = memnew(HtmlRect);
  this->add_child(this->m_rect);
  this->m_rect->set_owner(this->get_parent());
}

void CesiumGDCreditSystem::_bind_methods() {
  
}
