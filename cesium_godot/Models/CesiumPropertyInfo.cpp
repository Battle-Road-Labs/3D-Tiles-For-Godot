#include "CesiumPropertyInfo.h"

godot::Variant CesiumPropertyInfo::get_value() {
	return this->data;
}

godot::String CesiumPropertyInfo::get_string_value() {
	return godot::String(this->data);
}

void CesiumPropertyInfo::_bind_methods() {
	ClassDB::bind_method(D_METHOD("get_value"), &CesiumPropertyInfo::get_value);
	ClassDB::bind_method(D_METHOD("get_string_value"), &CesiumPropertyInfo::get_string_value);
}
