#include "CesiumGDCreditSystem.h"
#include "godot_cpp/classes/scene_tree.hpp"


CesiumGDCreditSystem* CesiumGDCreditSystem::get_singleton() {
  if (s_instance != nullptr) {
    return s_instance;
  }
  s_instance = memnew(CesiumGDCreditSystem);
  //TODO: Add to the scene tree
}
