#include "missing_functions.hpp"
#include "html_rect.hpp"

#include "Ultralight/String.h"
#include "godot/ultralight_singleton/ultralight_singleton.hpp"

#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

#include "convert/convert.hpp"

#include <JavaScriptCore/JSRetainPtr.h>

using namespace godot;

void HtmlRect::_bind_methods()
{
	ClassDB::bind_method(D_METHOD("get_index"), &HtmlRect::get_index);
	ClassDB::bind_method(D_METHOD("set_index", "p_index"), &HtmlRect::set_index);
	ClassDB::bind_method(D_METHOD("get_html"), &HtmlRect::get_html);
  ClassDB::bind_method(D_METHOD("set_html", "p_html"), &HtmlRect::set_html);
	        
	ClassDB::add_property(get_class_static(), PropertyInfo(Variant::STRING, "index_path"), "set_index", "get_index");
	
	ClassDB::add_property(get_class_static(), PropertyInfo(Variant::STRING, "html"), "set_html", "get_html");
}

HtmlRect::HtmlRect()
{
    CreateView();
}

HtmlRect::~HtmlRect() {}

void HtmlRect::CreateView()
{
    Vector2 size = get_size();

    ViewConfig view_config;
    view_config.is_accelerated = false;
    view_config.is_transparent = true;

    RefPtr<View> view = UltralightSingleton::get_singleton()->CreateView((int)size.x, (int)size.y, view_config, nullptr);

    if(view)
    { 
        SetView(view);
        LoadIndex(view);
    }
    
}

void HtmlRect::StoreGlobalObject(JSContextRef context, Dictionary obj)
{
    JSObjectRef godot_obj = JSValueToObject(context, Convert::ToJSValue(context, obj), NULL);

    // Get the global JavaScript object (aka 'window')
    JSObjectRef globalObj = JSContextGetGlobalObject(context);

    // Create a JavaScript String containing the name of our callback.
    JSRetainPtr<JSStringRef> propertyName = adopt(JSStringCreateWithUTF8CString("godot"));

    // Store our function in the page's global JavaScript object.
    JSObjectSetProperty(context, globalObj, propertyName.get(), godot_obj, 0, 0);
}

void HtmlRect::LoadIndex(RefPtr<View> view)
{
    if(index_path.is_empty()) {
        view->LoadHTML("<h1>Placeholder Text</h1>");
    } else
    {
        auto path_parts = index_path.split("://");
        if(path_parts.size() == 1) 
        {
            view->LoadURL(("file:///"+index_path).utf8().get_data());
        }
        else
        {
            view->LoadURL(index_path.utf8().get_data());
        }
    }
}


void HtmlRect::LoadHtml(const char* cstr, size_t length) {
    this->GetView()->LoadHTML(ultralight::String(cstr, length));
}


void HtmlRect::set_html(const godot::String html) {
    this->m_html = std::string(html.utf8().get_data(), html.length());
    this->LoadHtml(this->m_html.c_str(), this->m_html.size());
}

godot::String HtmlRect::get_html() const {
    return this->m_html.c_str();
}

void HtmlRect::set_index(const String p_index)
{
	index_path = p_index;
    LoadIndex(GetView());
}

godot::String HtmlRect::get_index() const
{
	return index_path;
}

Dictionary HtmlRect::call_on_dom_ready(const String &url)
{
    return _on_dom_ready(url);
}

void HtmlRect::_enter_tree() {
    if (!is_editor_mode()) return;
    
    this->set_custom_minimum_size(Vector2(1000, 150));
    
    this->set_anchors_preset(Control::LayoutPreset::PRESET_BOTTOM_LEFT);
    this->set_offset(Side::SIDE_TOP, -150.0f);
}

void HtmlRect::OnDOMReady(ultralight::View *caller, uint64_t frame_id, bool is_main_frame,
                          const ultralight::String &url)
{
    // Acquire the JS execution context for the current page.
    auto scoped_context = caller->LockJSContext();
    
    // Typecast to the underlying JSContextRef.
    JSContextRef context = (*scoped_context);
    
    String godot_url = godot::String(url.utf8().data());
    Dictionary godot_obj_dict = call_on_dom_ready(godot_url);

    StoreGlobalObject(context, godot_obj_dict);
}
