#include "register_types.hpp"

#include "godot/ultralight_singleton/ultralight_singleton.hpp"

#include "godot/view_rect/view_rect.hpp"
#include "godot/html_rect/html_rect.hpp"
#include "godot/inspector_rect/inspector_rect.hpp"
#include "app_example.hpp"

#include <godot_cpp/classes/engine.hpp>

using namespace godot;

static UltralightSingleton* ultralight_singleton = nullptr;

void initialize_html_module(ModuleInitializationLevel p_level) {
	if (p_level != MODULE_INITIALIZATION_LEVEL_SCENE) {
		return;
	}

    GDREGISTER_CLASS(UltralightSingleton);
    ultralight_singleton = memnew(UltralightSingleton); // memnew is super important.
    Engine::get_singleton()->register_singleton("UltralightSingleton", ultralight_singleton);

    GDREGISTER_ABSTRACT_CLASS(ViewRect);
    GDREGISTER_CLASS(HtmlRect);
    GDREGISTER_CLASS(InspectorRect);
    GDREGISTER_CLASS(AppExample);

}

void uninitialize_html_module(ModuleInitializationLevel p_level) {
	if (p_level != MODULE_INITIALIZATION_LEVEL_SCENE) {
		return;
	}

    Engine::get_singleton()->unregister_singleton("UltralightSingleton");
    delete ultralight_singleton;
}

extern "C" {
    // Initialization.
    GDExtensionBool GDE_EXPORT html_library_init(GDExtensionInterfaceGetProcAddress p_get_proc_address, const GDExtensionClassLibraryPtr p_library, GDExtensionInitialization *r_initialization) {
        godot::GDExtensionBinding::InitObject init_obj(p_get_proc_address, p_library, r_initialization);

        init_obj.register_initializer(initialize_html_module);
        init_obj.register_terminator(uninitialize_html_module);
        init_obj.set_minimum_library_initialization_level(MODULE_INITIALIZATION_LEVEL_SCENE);
        return init_obj.init();
    }
}