#ifndef CESIUM_GD_PANEL
#define CESIUM_GD_PANEL

#if defined(CESIUM_GD_EXT)
#include <godot_cpp/classes/node3d.hpp>
using namespace godot;
#elif defined(CESIUM_GD_MODULE)
#include "scene/3d/node_3d.h"
#endif

class CesiumGDPanel : public Object {
	GDCLASS(CesiumGDPanel, Object)
public:
	static void open_learn_page();

	static void open_help_page();

protected:
	static void _bind_methods();

};

#endif // !CESIUM_GD_PANEL
