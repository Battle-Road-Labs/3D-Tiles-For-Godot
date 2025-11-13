#include "executable_node.hpp"
#include "executable_control.hpp"

#if defined (CESIUM_GD_EXT)
#include "register_types.h"
#else
// Awful
#include "../register_types.h"
#endif


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

#if defined(CESIUM_GD_EXT)
#include <gdextension_interface.h>
#include <godot_cpp/core/defs.hpp>
#include <godot_cpp/core/class_db.hpp>
#include "godot_cpp/classes/engine.hpp"
using namespace godot;
#else
#include <core/object/class_db.h>
#endif


void initialize_cesium_godot_module(ModuleInitializationLevel p_level) {
	//We will probably have to switch the module initialization level to the editor
	//But we will keep it to the Scene for some testing that the library has been properly linked
	if (p_level != ModuleInitializationLevel::MODULE_INITIALIZATION_LEVEL_SCENE)
		return;
	//We will have to register all external classes to the class DB (we probably don't want this for common DataStructures, but rather Nodes)
	ClassDB::register_class<ExecutableControl>();
	ClassDB::register_class<ExecutableNode>();
	ClassDB::register_class<ExecutableNode3D>();
	ClassDB::register_class<ExecutableMeshInstance3D>();
	ClassDB::register_class<CesiumGeoreference>();
	ClassDB::register_class<Cesium3DTileset>();
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
	ClassDB::bind_integer_constant("CesiumGeoreference", "OriginType", "CartographicOrigin", (int32_t)CesiumGeoreference::OriginType::CartographicOrigin);
	ClassDB::bind_integer_constant("CesiumGeoreference", "OriginType", "TrueOrigin", (int32_t)CesiumGeoreference::OriginType::TrueOrigin);
}

void uninitialize_cesium_godot_module(ModuleInitializationLevel p_level) {
	//Hey there, hello, we don't do anything here actually
}

#if defined(CESIUM_GD_EXT)
extern "C" {
	GDExtensionBool GDE_EXPORT test_cesium_init(GDExtensionInterfaceGetProcAddress p_get_proc_address, const GDExtensionClassLibraryPtr p_library, GDExtensionInitialization* r_initialization){
		printf("%s", "Initialization of GDExtension");
		godot::GDExtensionBinding::InitObject init_obj(p_get_proc_address, p_library, r_initialization);
		init_obj.register_initializer(initialize_cesium_godot_module);
		init_obj.register_terminator(uninitialize_cesium_godot_module);
		init_obj.set_minimum_library_initialization_level(MODULE_INITIALIZATION_LEVEL_SCENE);
		return init_obj.init();
  }
}
#endif
