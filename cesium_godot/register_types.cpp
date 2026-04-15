#include "register_types.h"


#include "Implementations/DocumentContainer.h"
#include "Models/Cesium3DTile.h"
#include "Models/CesiumGDCreditSystem.h"
#include "Models/CesiumGDTileset.h"
#include "Models/CesiumHTTPRequestNode.h"
#include "Models/GeoreferencedNode.h"
#include "Utils/CesiumDebugUtils.h"
#include "Models/CesiumGlobe.h"
#include "Models/CesiumGDRasterOverlay.h"
#include "Models/CesiumGDPanel.h"
#include "Models/CesiumGDConfig.h"
#include "Utils/CesiumGDAssetBuilder.h"
#include "Utils/TokenTroubleShooting.h"
#include <cstdio>

#if defined(CESIUM_GD_EXT)
#include "godot_cpp/classes/engine.hpp"
#include <gdextension_interface.h>
#include <godot_cpp/core/defs.hpp>
#include <godot_cpp/core/class_db.hpp>
using namespace godot;
#elif defined(CESIUM_GD_MODULE)
#include <core/object/class_db.h>
#endif


void initialize_cesium_godot_module(ModuleInitializationLevel p_level) {
	printf("[CESIUM] initialize_cesium_godot_module called with level %d\n", (int)p_level);
	fflush(stdout);
	if (p_level != ModuleInitializationLevel::MODULE_INITIALIZATION_LEVEL_SCENE)
		return;
	printf("[CESIUM] Registering classes...\n");
	fflush(stdout);
	ClassDB::register_class<CesiumGeoreference>();
	printf("[CESIUM] registered CesiumGeoreference\n"); fflush(stdout);
	ClassDB::register_class<Cesium3DTileset>();
	printf("[CESIUM] registered Cesium3DTileset\n"); fflush(stdout);
	ClassDB::register_class<CesiumHTTPRequestNode>();
	ClassDB::register_class<CesiumDebugUtils>();
	ClassDB::register_class<CesiumGDPanel>();
	ClassDB::register_class<CesiumIonRasterOverlay>();
	ClassDB::register_class<CesiumGDConfig>();
	ClassDB::register_class<DocumentContainer>();
	ClassDB::register_class<CesiumGDAssetBuilder>();
	ClassDB::register_class<TokenTroubleshooting>();
	ClassDB::register_class<GeoreferencedMesh>();
	ClassDB::register_class<Cesium3DTile>();
	ClassDB::register_class<CesiumGDCreditSystem>(true);
	printf("[CESIUM] All classes registered\n");
	fflush(stdout);
}

void uninitialize_cesium_godot_module(ModuleInitializationLevel p_level) {
	//Hey there, hello, we don't do anything here actually
}

#if defined(CESIUM_GD_EXT)
extern "C" {
	GDExtensionBool GDE_EXPORT test_cesium_init(GDExtensionInterfaceGetProcAddress p_get_proc_address, const GDExtensionClassLibraryPtr p_library, GDExtensionInitialization* r_initialization){
		printf("[CESIUM] test_cesium_init entered\n");
		fflush(stdout);
		godot::GDExtensionBinding::InitObject init_obj(p_get_proc_address, p_library, r_initialization);
		printf("[CESIUM] InitObject created\n");
		fflush(stdout);
		init_obj.register_initializer(initialize_cesium_godot_module);
		printf("[CESIUM] initializer registered\n");
		fflush(stdout);
		init_obj.register_terminator(uninitialize_cesium_godot_module);
		printf("[CESIUM] terminator registered\n");
		fflush(stdout);
		init_obj.set_minimum_library_initialization_level(MODULE_INITIALIZATION_LEVEL_SCENE);
		printf("[CESIUM] calling init_obj.init()\n");
		fflush(stdout);
		GDExtensionBool result = init_obj.init();
		printf("[CESIUM] init returned: %d\n", result);
		fflush(stdout);
		return result;
  }
}
#endif
