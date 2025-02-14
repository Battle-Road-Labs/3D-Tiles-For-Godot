#include "inspector_rect.hpp"

#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

using namespace godot;

void InspectorRect::_bind_methods()
{
	ClassDB::bind_method(D_METHOD("get_html_rect"), &InspectorRect::get_html_rect);
	ClassDB::bind_method(D_METHOD("set_html_rect", "p_html_rect"), &InspectorRect::set_html_rect);
	ClassDB::add_property("InspectorRect", PropertyInfo(Variant::OBJECT, "html_rect", PROPERTY_HINT_NODE_TYPE), "set_html_rect", "get_html_rect");
}

InspectorRect::InspectorRect() { }

InspectorRect::~InspectorRect() { }

void InspectorRect::_process(double delta)
{
    ViewRect::_process(delta);
    if(html_rect != nullptr)
    {
        auto html_rect_view = html_rect->GetView();
        if(html_rect_view)
        {
            auto view = GetView();
            auto html_rect_inspector_view = html_rect->GetInspectorView();
            if(!html_rect_inspector_view)
            {
                if(view) SetView(nullptr);
                html_rect_view->CreateLocalInspectorView();
            }
            else if(view != html_rect_inspector_view)
            {
                auto size = get_size();
                html_rect_inspector_view->Resize((int)size.x, (int)size.y);
                SetView(html_rect_inspector_view);
            }
        }
    }
}


void InspectorRect::set_html_rect(HtmlRect* p_html_rect)
{
    html_rect = p_html_rect;
}

HtmlRect* InspectorRect::get_html_rect()
{
    return html_rect;
}
