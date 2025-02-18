#ifndef HTMLRECT_H
#define HTMLRECT_H

#include "Ultralight/String.h"
#include "godot/view_rect/view_rect.hpp"

#include <godot_cpp/core/binder_common.hpp>
#include <godot_cpp/classes/main_loop.hpp>

namespace godot {
    class HtmlRect : public ViewRect {
        GDCLASS(HtmlRect, ViewRect);

        private:
            void CreateView();
            void StoreGlobalObject(JSContextRef context, Dictionary obj);

            
        protected:
            static void _bind_methods();
            
            Dictionary call_on_dom_ready(const String &url);
            virtual Dictionary _on_dom_ready(const String &url) { return Dictionary(); };

        public:
            ultralight::String m_html;
            String index_path = "";
            HtmlRect();
            ~HtmlRect();

            // void _process(double delta) override;
            // void _gui_input(const Ref<InputEvent> &event) override;

            void LoadIndex(ultralight::RefPtr<ultralight::View> view);

            void LoadHtml();

            void _enter_tree() override;
            
            void set_index(const String p_index);
        
            String get_index() const;

            void set_html(const ultralight::String& html);
            
            const ultralight::String& get_html() const;
            
            void OnDOMReady(ultralight::View *caller, uint64_t frame_id, bool is_main_frame, const ultralight::String &url) override;
    };
}

#endif
