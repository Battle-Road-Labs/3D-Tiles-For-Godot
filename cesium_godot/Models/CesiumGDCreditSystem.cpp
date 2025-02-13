#include "CesiumGDCreditSystem.h"
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
  return s_instance;
}


void CesiumGDCreditSystem::set_credit_system(std::shared_ptr<CesiumUtility::CreditSystem> creditSystem) {
  this->m_creditSystem = creditSystem;
}
