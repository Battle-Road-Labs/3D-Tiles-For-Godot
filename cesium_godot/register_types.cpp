#include "register_types.h"


#include "Models/CesiumGDCreditSystem.h"
#include "Models/CesiumGDTileset.h"
#include "Models/CesiumHTTPRequestNode.h"
#include "Utils/CesiumDebugUtils.h"
#include "Models/CesiumGlobe.h"
#include "Models/CesiumGDRasterOverlay.h"
#include "Models/CesiumGDPanel.h"
#include "Models/CesiumGDConfig.h"
#include "Models/CesiumGDGeoreference.h"
#include "Utils/CesiumGDAssetBuilder.h"
#include "app_example.hpp"
#include "godot/inspector_rect/inspector_rect.hpp"
#include "godot/ultralight_singleton/ultralight_singleton.hpp"
#include "godot_cpp/classes/engine.hpp"

#if defined(CESIUM_GD_EXT)
#include <gdextension_interface.h>
#include <godot_cpp/core/defs.hpp>
#include <godot_cpp/core/class_db.hpp>
using namespace godot;
#elif defined(CESIUM_GD_MODULE)
#include <core/object/class_db.h>
#endif

static UltralightSingleton* ultralight_singleton = nullptr;

void initialize_cesium_godot_module(ModuleInitializationLevel p_level) {
	//We will probably have to switch the module initialization level to the editor
	//But we will keep it to the Scene for some testing that the library has been properly linked
	if (p_level != ModuleInitializationLevel::MODULE_INITIALIZATION_LEVEL_SCENE)
		return;
	//We will have to register all external classes to the class DB (we probably don't want this for common DataStructures, but rather Nodes)
	ClassDB::register_class<CesiumGlobe>();
	ClassDB::register_class<CesiumGDGeoreference>();
	ClassDB::register_class<CesiumGDTileset>();
	ClassDB::register_class<CesiumHTTPRequestNode>();
	ClassDB::register_class<CesiumDebugUtils>();
	ClassDB::register_class<CesiumGDPanel>();
	ClassDB::register_class<CesiumGDRasterOverlay>();
	ClassDB::register_class<CesiumGDConfig>();
	ClassDB::register_class<CesiumGDAssetBuilder>();
	
	GDREGISTER_CLASS(UltralightSingleton);
  ultralight_singleton = memnew(UltralightSingleton); // memnew is super important.
  Engine::get_singleton()->register_singleton("UltralightSingleton", ultralight_singleton);

  GDREGISTER_ABSTRACT_CLASS(ViewRect);
  GDREGISTER_CLASS(HtmlRect);
  GDREGISTER_CLASS(InspectorRect);
  GDREGISTER_CLASS(AppExample);
	ClassDB::register_class<CesiumGDCreditSystem>();
}

void uninitialize_cesium_godot_module(ModuleInitializationLevel p_level) {
	//Hey there, hello, we don't do anything here actually
  Engine::get_singleton()->unregister_singleton("UltralightSingleton");
  delete ultralight_singleton;
}

extern "C" {
	  GDExtensionBool GDE_EXPORT test_cesium_init(GDExtensionInterfaceGetProcAddress p_get_proc_address, const GDExtensionClassLibraryPtr p_library, GDExtensionInitialization* r_initialization){
    godot::GDExtensionBinding::InitObject init_obj(p_get_proc_address, p_library, r_initialization);
    init_obj.register_initializer(initialize_cesium_godot_module);
    init_obj.register_terminator(uninitialize_cesium_godot_module);
    init_obj.set_minimum_library_initialization_level(MODULE_INITIALIZATION_LEVEL_SCENE);
    return init_obj.init();
  }
}

