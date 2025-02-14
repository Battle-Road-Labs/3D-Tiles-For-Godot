#include "app_example.hpp"

#include <godot_cpp/core/class_db.hpp>

#include <godot_cpp/variant/utility_functions.hpp>

using namespace godot;

void AppExample::_bind_methods() {}

Dictionary AppExample::_on_dom_ready(const godot::String &url)
{
    auto obj = Dictionary();
    obj["url"] = url;
    return obj;
}
