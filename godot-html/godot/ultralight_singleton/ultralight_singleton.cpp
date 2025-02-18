#include "ultralight_singleton.hpp"

#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/classes/rendering_server.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

using namespace godot;

UltralightSingleton *UltralightSingleton::singleton = nullptr;

void UltralightSingleton::_bind_methods()
{
    ClassDB::bind_method(D_METHOD("RenderFrame"), &UltralightSingleton::RenderFrame);
}

UltralightSingleton::UltralightSingleton()
{
	ERR_FAIL_COND(singleton != nullptr);
    singleton = this;
}

UltralightSingleton::~UltralightSingleton()
{
    ERR_FAIL_COND(singleton != this);
    singleton = nullptr;

    if(is_setup)
    {
        RenderingServer::get_singleton()->disconnect("frame_pre_draw", Callable(this, "RenderFrame"));
        delete manager;
    }
}

UltralightSingleton *UltralightSingleton::get_singleton()
{
    if(!singleton->is_setup)
    {
        singleton->manager = new GodotHTML::UltralightManager();

        RenderingServer::get_singleton()->connect("frame_pre_draw", Callable(singleton, "RenderFrame"));
        singleton->is_setup = true;
    }
    return singleton;
}

void UltralightSingleton::RenderFrame()
{
    if(manager) manager->RenderFrame();
}

RefPtr<View> godot::UltralightSingleton::CreateView(uint32_t width, uint32_t height, const ViewConfig &config, RefPtr<Session> session)
{
    return GodotHTML::UltralightManager::GetSingleton()->GetRenderer()->CreateView(std::max(width, (uint32_t)1), std::max(height, (uint32_t)1), config, nullptr);
}
