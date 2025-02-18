#include "ultralight_manager.hpp"
using namespace godot;

#include <AppCore/Platform.h>

namespace GodotHTML
{

UltralightManager* GodotHTML::UltralightManager::singleton = nullptr;

UltralightManager::UltralightManager() {
    InitPlatform();
    CreateRenderer();
    singleton = this;
};

UltralightManager::~UltralightManager() {
    if(file_system != nullptr)
    {
        delete file_system;
        file_system = nullptr;
    }
    if(clipboard != nullptr)
    {
        delete clipboard;
        clipboard = nullptr;
    }
    singleton = nullptr;
}

UltralightManager *GodotHTML::UltralightManager::GetSingleton()
{
    return singleton;
}

void UltralightManager::InitPlatform()
{
    ultralight::Config config;
    config.resource_path_prefix = "addons/cesium_godot/resources/";
    Platform::instance().set_config(config);

    ///
    /// Use the OS's native font loader
    ///
    Platform::instance().set_font_loader(GetPlatformFontLoader());

    file_system = new GodotFileSystem();
    Platform::instance().set_file_system(file_system);

    clipboard = new GodotClipboard();
    Platform::instance().set_clipboard(clipboard);

    ///
    /// Use the default logger (writes to a log file)
    ///
    Platform::instance().set_logger(GetDefaultLogger("ultralight.log"));
}

void UltralightManager::CreateRenderer()
{
    ///
    /// Create our Renderer (call this only once per application).
    /// 
    /// The Renderer singleton maintains the lifetime of the library
    /// and is required before creating any Views.
    ///
    /// You should set up the Platform handlers before this.
    ///
    renderer = Renderer::Create();
}

RefPtr<Renderer> UltralightManager::GetRenderer() {
    return renderer;
}

void UltralightManager::UpdateLogic() {
    renderer->Update();
}

void UltralightManager::RenderFrame() {
    renderer->Render();
    renderer->RefreshDisplay(0);
}

}
