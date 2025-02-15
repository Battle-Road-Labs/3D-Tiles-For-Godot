#include "view_rect.hpp"

#include "godot/ultralight_singleton/ultralight_singleton.hpp"
#include "godot_cpp/classes/resource_saver.hpp"

#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/classes/rendering_server.hpp>
#include <godot_cpp/variant/packed_byte_array.hpp>

#include <godot_cpp/classes/os.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

using namespace godot;

#pragma region ViewRect Public methods.

ViewRect::ViewRect()
{
    // #region Set default values.
    set_mouse_filter(MouseFilter::MOUSE_FILTER_STOP);
    set_focus_mode(FocusMode::FOCUS_CLICK);
    set_force_pass_scroll_events(false);
    // #endregion

    image = Ref<Image>();

    RenderingServer::get_singleton()->connect("frame_post_draw", Callable(this, "render_frame"));
    connect("resized", Callable(this, "size_changed"));
}

ViewRect::~ViewRect()
{
    RenderingServer::get_singleton()->disconnect("frame_post_draw", Callable(this, "render_frame"));
    disconnect("resized", Callable(this, "size_changed"));
}

void ViewRect::SetView(RefPtr<View> p_view)
{
    if(p_view)
    {
        if(view)
        {
            view->set_view_listener(nullptr);
            view->set_load_listener(nullptr);
        }
        view = p_view;
        view->set_view_listener(this);
        view->set_load_listener(this);
    }
}

RefPtr<View> ViewRect::GetView()
{
    return view;
}

RefPtr<View> ViewRect::GetInspectorView()
{
    return inspector_view;
}

#pragma endregion

#pragma region Godot methods.

void ViewRect::_bind_methods()
{
    ClassDB::bind_method(D_METHOD("render_frame"), &ViewRect::RenderFrame);
    ClassDB::bind_method(D_METHOD("size_changed"), &ViewRect::SizeChanged);
}

void ViewRect::_process(double delta)
{
    if(!view) return;
    ///
    /// Give the library a chance to handle any pending tasks and timers.
    ///
    ///
    GodotHTML::UltralightManager::GetSingleton()->UpdateLogic();
    printf("Update logic!\n");
    for (; !events.empty(); events.pop())
    {
        switch(events.front())
        {
            case Event::mouse:
                view->FireMouseEvent(mouse_events.front());
                mouse_events.pop();
                break;
            case Event::key:
                view->FireKeyEvent(key_events.front());
                key_events.pop();
                break;
            case Event::scroll:
                view->FireScrollEvent(scroll_events.front());
                scroll_events.pop();
                break;
        }
    }
}

void ViewRect::_gui_input(const Ref<InputEvent> &event)
{
    if(auto e = dynamic_cast<InputEventMouseButton*>(*event))
    {
        HandleMouseButton(e);
    }
    else if(auto e = dynamic_cast<InputEventMouseMotion*>(*event))
    {
        HandleMouseMotion(e);
    }
    else if(auto e = dynamic_cast<InputEventKey*>(*event))
    {
        HandleKey(e);
    }
}

void ViewRect::_draw()
{
    if(image_texture.is_valid())
    {
        Vector2 size = get_size();
        printf("Drew image!\n");
        draw_texture_rect(image_texture, Rect2(0, 0, size.x, size.y), false);
        //ResourceSaver::get_singleton()->save(this->image_texture, "res://testTex.tres");
    }
}

#pragma endregion

#pragma region ViewRect Private methods.

void ViewRect::HandleMouseButton(InputEventMouseButton *event)
{
    MouseButton index = event->get_button_index();
    // Check if scroll.
    if(index == MouseButton::MOUSE_BUTTON_WHEEL_UP || index == MouseButton::MOUSE_BUTTON_WHEEL_DOWN)
    {
        ScrollEvent evt;
        evt.type = ScrollEvent::kType_ScrollByPixel;
        int delta = index == MouseButton::MOUSE_BUTTON_WHEEL_UP ? 1 : -1;
        evt.delta_x = delta * 10;
        evt.delta_y = delta * 10;
        scroll_events.push(evt);
        events.push(Event::scroll);
    }
    else if(index < 4)
    {
        auto pos = event->get_position();
        MouseEvent evt;
        evt.x = (int)pos.x;
        evt.y = (int)pos.y;
        evt.type = event->is_pressed() ? MouseEvent::kType_MouseDown : MouseEvent::kType_MouseUp;
        switch(index)
        {
            case MouseButton::MOUSE_BUTTON_LEFT:
                evt.button = MouseEvent::Button::kButton_Left;
                break;
            case MouseButton::MOUSE_BUTTON_MIDDLE:
                evt.button = MouseEvent::Button::kButton_Middle;
                break;
            case MouseButton::MOUSE_BUTTON_RIGHT:
                evt.button = MouseEvent::Button::kButton_Right;
                break;
        };
        mouse_events.push(evt);
        events.push(Event::mouse);
    }
}

void ViewRect::HandleMouseMotion(InputEventMouseMotion *event)
{
    auto pos = event->get_position();
    MouseEvent evt;
    evt.type = MouseEvent::kType_MouseMoved;
    evt.x = (int)pos.x;
    evt.y = (int)pos.y;
    evt.button = MouseEvent::kButton_None;
    mouse_events.push(evt);
    events.push(Event::mouse);
}

void ViewRect::HandleKey(InputEventKey *event)
{
    if(!event->is_pressed()) return;

    auto os = OS::get_singleton();

    int keycode = event->get_keycode();
    
    // If Unicode, push character to key_events.
    if(os->is_keycode_unicode(keycode))
    {
        KeyEvent evt;
        auto character = String::chr(event->get_unicode());
        evt.type = KeyEvent::kType_Char;
        evt.text = character.utf8().get_data();
        key_events.push(evt);
        events.push(Event::key);
    }

    // Always submit raw key events.
    KeyEvent evt;
    evt.type = KeyEvent::kType_RawKeyDown;
    auto event_it = KeyMap.find(keycode);
    if(event_it != KeyMap.end()) {
        // evt.native_key_code = event_it->second;
        evt.virtual_key_code = event_it->second;
    }
    // Convert godot modifiers to ultralight modifiers.
    auto modifiers = event->get_modifiers_mask();
    uint8_t ultralightModifier = 0;
    for (const auto& [godotMask, ultralightMask] : ModifierMap) {
        if (modifiers & godotMask) {
            ultralightModifier |= ultralightMask;
        }
    }
    evt.modifiers = ultralightModifier;

    // You'll need to generate a key identifier from the virtual key code
    // when synthesizing events. This function is provided in KeyEvent.h
    GetKeyIdentifierFromVirtualKeyCode(evt.virtual_key_code, evt.key_identifier);

    key_events.push(evt);
    events.push(Event::key);
}

void ViewRect::RenderFrame()
{
    if(!view) return;
    printf("Rendered frame!\n");
    ///
    /// Get the Surface as a BitmapSurface (the default implementation).
    ///
    BitmapSurface* surface = (BitmapSurface*)(view->surface());
    ///
    /// Check if our Surface is dirty (pixels have changed).
    ///
    if (!surface->dirty_bounds().IsEmpty()) {
        ///
        /// Psuedo-code to upload Surface's bitmap to GPU texture.
        ///
        auto tmpBitmap = Bitmap::Create(*surface->bitmap());
        CopyBitmapToTexture(tmpBitmap);
        
        ///
        /// Clear the dirty bounds.
        ///
        surface->ClearDirtyBounds();
    }
    queue_redraw();
}

void ViewRect::SizeChanged()
{
    if(!view) return;
    Vector2 size = get_size();
    view->Resize((int)size.x, (int)size.y);
    image = Ref<Image>();
}

void ViewRect::CopyBitmapToTexture(RefPtr<Bitmap> bitmap)
{
    bitmap->SwapRedBlueChannels();
    auto pixels = bitmap->LockPixelsSafe();
    PackedByteArray arr = PackedByteArray();
    printf("Expected pixels: %zu, bitmap size: (%d, %d)\n", pixels.size(), bitmap->width(), bitmap->height());
    arr.resize(pixels.size());
    memcpy(arr.ptrw(), pixels.data(), pixels.size());
    if(image.is_null())
    {
        image = Image::create_from_data(bitmap->width(), bitmap->height(), false, Image::FORMAT_RGBA8, arr);
        if(!image.is_valid() || image->is_empty()) return;
        image_texture = ImageTexture::create_from_image(image);
    }
    else
    {
        image->set_data(bitmap->width(), bitmap->height(), false, Image::Format::FORMAT_RGBA8, arr); 
        image_texture->update(image);
    }
}

#pragma endregion

#pragma region ViewListener Callbacks

RefPtr<View> ViewRect::OnCreateInspectorView(ultralight::View* caller, bool is_local, const ultralight::String& inspected_url)
{
    Vector2 size = get_size();

    ViewConfig view_config;
    view_config.is_accelerated = false;
    view_config.is_transparent = true;

    RefPtr<View> temp_view = UltralightSingleton::get_singleton()->CreateView((int)size.x, (int)size.y, view_config, nullptr);
    if(temp_view)
    {
        inspector_view = temp_view;
        return inspector_view;
    }
    return nullptr;
}

void ViewRect::OnChangeCursor(ultralight::View* caller, Cursor cursor)
{
    auto it = CursorMap.find(cursor);
    if(it != CursorMap.end()) {
        set_default_cursor_shape(it->second);
    }
}

#pragma endregion
