#ifndef INSPECTRECT_H
#define INSPECTRECT_H

#include "godot/view_rect/view_rect.hpp"
#include "godot/html_rect/html_rect.hpp"

namespace godot {

    class InspectorRect : public ViewRect {
        GDCLASS(InspectorRect, ViewRect);

        protected:
            static void _bind_methods();

        public:
            HtmlRect* html_rect = nullptr;

            InspectorRect();
            ~InspectorRect();

            void _process(double delta) override;
            // void _gui_input(const Ref<InputEvent> &event) override;

            void set_html_rect(HtmlRect* p_html_rect);
            HtmlRect* get_html_rect();
    };

}

#endif