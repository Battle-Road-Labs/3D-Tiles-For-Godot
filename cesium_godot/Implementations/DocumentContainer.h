#ifndef DOCUMENT_CONTAINER_H
#define DOCUMENT_CONTAINER_H

#ifdef __APPLE__
#include "Utils/GodotHttpClient.h"
#else
#include "Utils/CurlHttpClient.h"
#endif
#include "godot_cpp/classes/control.hpp"
#include "godot_cpp/classes/image_texture.hpp"
#include "godot_cpp/classes/wrapped.hpp"
#include "litehtml.h"
#include "litehtml/document.h"
#include <cstdint>
#include <unordered_map>

using namespace godot;

class DocumentContainer final : public Control, public litehtml::document_container {
	GDCLASS(DocumentContainer, Control)
public:

	void _draw() override;
	
	void set_html(const String& html);
	
	void set_html_stl(const std::string_view& html);
	
	litehtml::uint_ptr create_font(const litehtml::font_description& descr, const litehtml::document* doc, litehtml::font_metrics* fm) override;
	void delete_font(litehtml::uint_ptr hFont) override;
	int	text_width(const char* text, litehtml::uint_ptr hFont) override;
	void draw_text(litehtml::uint_ptr hdc, const char* text, litehtml::uint_ptr hFont, litehtml::web_color color, const litehtml::position& pos) override;
	int pt_to_px(int pt) const override;
	int get_default_font_size() const override;
	const char*	get_default_font_name() const override;
	void draw_list_marker(litehtml::uint_ptr hdc, const litehtml::list_marker& marker) override;
	void load_image(const char* src, const char* baseurl, bool redraw_on_ready) override;
	void get_image_size(const char* src, const char* baseurl, litehtml::size& sz) override;
	void draw_borders(litehtml::uint_ptr hdc, const litehtml::borders& borders, const litehtml::position& draw_pos, bool root) override;
	void draw_image(litehtml::uint_ptr hdc, const litehtml::background_layer& layer, const std::string& url, const std::string& base_url) override;
	void set_caption(const char* caption) override;
	void set_base_url(const char* base_url) override;
	void link(const std::shared_ptr<litehtml::document>& doc, const litehtml::element::ptr& el) override;
	void on_anchor_click(const char* url, const litehtml::element::ptr& el) override;
	void set_cursor(const char* cursor) override;
	void transform_text(litehtml::string& text, litehtml::text_transform tt) override;
	void import_css(litehtml::string& text, const litehtml::string& url, litehtml::string& baseurl) override;
	void set_clip(const litehtml::position& pos, const litehtml::border_radiuses& bdr_radius) override;
	void del_clip() override;	
	void draw_solid_fill(litehtml::uint_ptr hdc, const litehtml::background_layer& layer, const litehtml::web_color& color) override {};
	// void get_client_rect(litehtml::position& client) const override;
	litehtml::element::ptr	create_element( const char* tag_name,
													const litehtml::string_map& attributes,
													const std::shared_ptr<litehtml::document>& doc) override;
	void get_media_features(litehtml::media_features& media) const override;
	void get_language(litehtml::string& language, litehtml::string& culture) const override;
	litehtml::string resolve_color(const litehtml::string& /*color*/) const override { return litehtml::string(); }
	void split_text(const char* text, const std::function<void(const char*)>& on_word, const std::function<void(const char*)>& on_space) override;	
	
	void draw_linear_gradient(litehtml::uint_ptr hdc, const litehtml::background_layer& layer, const litehtml::background_layer::linear_gradient& gradient) override {}
	
	void draw_radial_gradient(litehtml::uint_ptr hdc, const litehtml::background_layer& layer, const litehtml::background_layer::radial_gradient& gradient) override {}
	

	void draw_conic_gradient(litehtml::uint_ptr hdc, const litehtml::background_layer& layer, const litehtml::background_layer::conic_gradient& gradient) override {}
	

	void on_mouse_event(const litehtml::element::ptr& el, litehtml::mouse_event event) override {}
	
	void get_viewport(litehtml::position& viewport) const override;
	
private:
	// We use a hash here bc we do not want to keep a copy of the string
	std::unordered_map<uint32_t, Ref<ImageTexture>> m_imageCache;
#ifdef __APPLE__
	GodotHttpClient<2> m_httpClient;
#else
	CurlHttpClient<2> m_httpClient;
#endif
	// I know this is "backwards" if you think abt this in OOP terms, but I beg you to think of this class as just functional implementations
	litehtml::document::ptr m_document = nullptr;

protected:
	static void _bind_methods();
	
};

#endif
