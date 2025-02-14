#include "CesiumGDCreditSystem.h"

#include "CesiumUtility/CreditSystem.h"
#include "godot_cpp/classes/engine.hpp"
#include "godot_cpp/classes/scene_tree.hpp"
#include "godot_cpp/classes/window.hpp"

CesiumGDCreditSystem* CesiumGDCreditSystem::get_singleton() {
  if (s_instance != nullptr) {
    return s_instance;
  }
  SceneTree* sceneTree = Object::cast_to<SceneTree>(Engine::get_singleton()->get_main_loop());
  Node* root = sceneTree->get_root();
  s_instance = memnew(CesiumGDCreditSystem);
  root->add_child(s_instance);
  s_instance->set_owner(root);
  s_instance->m_creditSystems.reserve(3); //Default of 3, I don't think we'll need more than 3 tilesets
  return s_instance;
}

void CesiumGDCreditSystem::_process(double p_delta) {
  this->update_credits();
}


void CesiumGDCreditSystem::update_credits() {
  if (!this->m_creditSystems.empty()) {
    this->m_html = "";
  }
  
  for (const auto& creditSystem : this->m_creditSystems) {
    const std::vector<CesiumUtility::Credit>& creditsToShow = creditSystem->getCreditsToShowThisFrame();
    for (const CesiumUtility::Credit& credit : creditsToShow) {
      if (!creditSystem->shouldBeShownOnScreen(credit)) {
        //TODO: Popup
        continue;
      }
      const std::string& html = creditSystem->getHtml(credit);
      this->m_html += html.c_str();
    }
  }
  this->LoadHtml(this->m_html);
}

void CesiumGDCreditSystem::add_credit_system(std::shared_ptr<CesiumUtility::CreditSystem> creditSystem) {
  this->m_creditSystems.emplace_back(creditSystem);
}



void CesiumGDCreditSystem::_bind_methods() {
  
}
