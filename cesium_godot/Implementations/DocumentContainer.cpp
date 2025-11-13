#include "DocumentContainer.h"
#if defined(CESIUM_GD_EXT)
#include "godot_cpp/classes/control.hpp"
#include "godot_cpp/classes/font.hpp"
#include "godot_cpp/classes/global_constants.hpp"
#include "godot_cpp/classes/image.hpp"
#include "godot_cpp/classes/image_texture.hpp"
#include "godot_cpp/classes/text_line.hpp"
#include "godot_cpp/core/error_macros.hpp"
#include "godot_cpp/core/memory.hpp"
#include "godot_cpp/variant/color.hpp"
#include "godot_cpp/variant/packed_byte_array.hpp"
#include "godot_cpp/variant/rect2.hpp"
#else

#include "scene/resources/font.h"
#endif
#include "litehtml/background.h"
#include "litehtml/document.h"
#include "litehtml/types.h"
#include "litehtml/web_color.h"
#include <cstdint>
#include <cstdio>
#include <memory>


constexpr inline uint32_t HashFnv1a(const char *data, const uint32_t length)
{
	// NOLINTNEXTLINE
	uint32_t hash = 0x811c9dc5;
	const uint32_t prime = 0x1000193;

	for (uint32_t i = 0; i < length; ++i)
	{
		uint8_t value = data[i];
		hash = hash ^ value;
		hash *= prime;
	}

	return hash;
}


constexpr inline uint32_t HashFnv1a(const std::string_view &data)
{
	return HashFnv1a(data.data(), static_cast<uint32_t>(data.size()));
}


struct FontHandle {
	Ref<Font> font;
	float ascent;
	float descent;
	float height;
};


const char*	DocumentContainer::get_default_font_name() const {
	return Control::get_theme_default_font()->get_font_name().utf8().get_data();
}


void DocumentContainer::set_html_stl(const std::string_view& html) {
	this->m_document = litehtml::document::createFromString(html.data(), this);
	constexpr int32_t MAX_WIDTH = 1000;
	int32_t currentWidth = this->m_document->render(MAX_WIDTH);
	this->set_size(Vector2(currentWidth, this->get_size().height));
	this->queue_redraw();
}

void DocumentContainer::set_html(const String& html) {
	this->m_document = litehtml::document::createFromString(html.utf8().get_data(), this);
	this->m_document->render(this->get_size().width);
	this->queue_redraw();
}

void DocumentContainer::_draw() {
	if (this->m_document == nullptr) return;
	litehtml::position origin(
		0,
		0,
		this->get_size().width,
		this->get_size().height
	);
	this->m_document->draw((litehtml::uint_ptr)this, 0, 0, &origin);
}


litehtml::uint_ptr DocumentContainer::create_font(const litehtml::font_description& descr, const litehtml::document* doc, litehtml::font_metrics* fm) {
	FontHandle* handle = new FontHandle();
	handle->font = Control::get_theme_default_font();
	handle->ascent = handle->font->get_ascent(descr.size);
	handle->descent = handle->font->get_descent(descr.size);
	handle->height = handle->font->get_height(descr.size);

	if (fm != nullptr) {
		fm->ascent = handle->ascent;
		fm->descent = handle->descent;
		fm->height = handle->height;
		fm->x_height = handle->height * 0.5f;
	}
	
	return reinterpret_cast<litehtml::uint_ptr>(handle);
}


void DocumentContainer::delete_font(litehtml::uint_ptr hFont) {
	FontHandle* handle = reinterpret_cast<FontHandle*>(hFont);
	delete handle;
}

int	DocumentContainer::text_width(const char* text, litehtml::uint_ptr hFont) {
	FontHandle* handle = reinterpret_cast<FontHandle*>(hFont);
	return handle->font->get_string_size(text).x;
}

int DocumentContainer::pt_to_px(int pt) const {
	constexpr float PT_TO_PX_FACTOR = (72.0f / 96.0f);
	return pt * PT_TO_PX_FACTOR; // Narrowing conversion, but that's fine
}

int DocumentContainer::get_default_font_size() const {
	return 16;
}


void DocumentContainer::draw_image(litehtml::uint_ptr hdc, const litehtml::background_layer& layer, const std::string& url, const std::string& base_url) {
	uint32_t hash = HashFnv1a(url.c_str());
	if (this->m_imageCache.find(hash) == this->m_imageCache.end()) {
		// WARN_PRINT("Image not found cache!");
		return;
	}
	const Ref<ImageTexture>& texture = this->m_imageCache.at(hash);
	Rect2 destinationRect(
		layer.origin_box.x,
		layer.origin_box.y,
		texture->get_width(),
		texture->get_height()
	);
	this->draw_texture_rect(texture, destinationRect, false);
}

void DocumentContainer::load_image(const char* src, const char* baseurl, bool redraw_on_ready) {
	std::string fetchUrl = src;
	if (this->m_imageCache.find(HashFnv1a(fetchUrl)) != this->m_imageCache.end()) {
		return;
	}
	this->m_httpClient.send_get_same_thread(src, [fetchUrl, this](int32_t status, const PackedByteArray& body) {
        if (status >= HTTPClient::ResponseCode::RESPONSE_BAD_REQUEST) {
			std::string tmpStr(reinterpret_cast<const char*>(body.ptr()), body.size());
	        String bodyStr = tmpStr.c_str();
        	// ERR_PRINT(String("Not able to load image from ") + String(fetchUrl.c_str()) + String(", response: ") + bodyStr);
        	return;
        }

		// All of the images we need to handle are most likely png
		Ref<Image> image = memnew(Image);
		Error err = image->load_png_from_buffer(body);
		
		if (err != Error::OK) {
			// ERR_PRINT(String("Failed to parse image, possible bad format, source: ") + String(fetchUrl.c_str()));
			return;
		}
		
		// Create an ImageTexture based on the data
		Ref<ImageTexture> texture = ImageTexture::create_from_image(image);
		
        // We do copy the data, but that's fine
        this->m_imageCache.insert_or_assign(HashFnv1a(fetchUrl), texture);
	}, {});
}


void DocumentContainer::draw_text(litehtml::uint_ptr hdc, const char* text, litehtml::uint_ptr hFont, litehtml::web_color color, const litehtml::position& pos) {
	FontHandle* fontHandle = reinterpret_cast<FontHandle*>(hFont);
	if (color == litehtml::web_color::black) {
		color = litehtml::web_color::white;
	}
	Color textColor(color.red / 255.0f, color.green / 255.0f, color.blue / 255.0f, color.alpha / 255.0f);
	this->draw_string(fontHandle->font, Vector2(pos.x, pos.y + fontHandle->ascent), text, (HorizontalAlignment)0, -1, get_default_font_size(), textColor);
}


void DocumentContainer::get_image_size(const char* src, const char* baseurl, litehtml::size& sz) {
	if (this->m_imageCache.find(HashFnv1a(src)) == this->m_imageCache.end()) {
		return;
	}
	const Ref<ImageTexture>& image = this->m_imageCache.at(HashFnv1a(src));
	sz.width = image->get_size().width;
	sz.height = image->get_size().height;
}

void DocumentContainer::draw_borders(litehtml::uint_ptr hdc, const litehtml::borders& borders, const litehtml::position& draw_pos, bool root) {
	(void)root;
}

void DocumentContainer::set_caption(const char* caption) {
	
}

void DocumentContainer::set_base_url(const char* base_url) {
	
}

void DocumentContainer::link(const std::shared_ptr<litehtml::document>& doc, const litehtml::element::ptr& el) {
	
}

void DocumentContainer::on_anchor_click(const char* url, const litehtml::element::ptr& el) {
	
}

void DocumentContainer::set_cursor(const char* cursor) {
	
}

void DocumentContainer::transform_text(litehtml::string& text, litehtml::text_transform tt) {
	
}

void DocumentContainer::import_css(litehtml::string& text, const litehtml::string& url, litehtml::string& baseurl) {
	
}

void DocumentContainer::set_clip(const litehtml::position& pos, const litehtml::border_radiuses& bdr_radius) {
	
}

void DocumentContainer::del_clip() {
	
}

void DocumentContainer::get_media_features(litehtml::media_features& media) const {
	media.type = litehtml::media_type_screen;
	media.width = get_size().x;
	media.height = get_size().y;
	media.device_width = get_size().x;
	media.device_height = get_size().y;
	media.color = true;
	media.monochrome = false;
	media.resolution = 96;
}

void DocumentContainer::get_language(litehtml::string& language, litehtml::string& culture) const {
	language = "en";
	culture = "us";	
}

void DocumentContainer::split_text(const char* text, const std::function<void(const char*)>& on_word, const std::function<void(const char*)>& on_space) {
	// Reimplement the original split_text
	std::u32string str;
	std::u32string str_in = (const char32_t*)litehtml::utf8_to_utf32(text);
	for (auto c : str_in)
	{
		if (c <= ' ' && (c == ' ' || c == '\t' || c == '\n' || c == '\r' || c == '\f'))
		{
			if (!str.empty())
			{
				on_word(litehtml::utf32_to_utf8(str));
				str.clear();
			}
			str += c;
			on_space(litehtml::utf32_to_utf8(str));
			str.clear();
		}
		// CJK character range
		else if (c >= 0x4E00 && c <= 0x9FCC)
		{
			if (!str.empty())
			{
				on_word(litehtml::utf32_to_utf8(str));
				str.clear();
			}
			str += c;
			on_word(litehtml::utf32_to_utf8(str));
			str.clear();
		}
		else
		{
			str += c;
		}
	}
	if (!str.empty())
	{
		on_word(litehtml::utf32_to_utf8(str));
	}}


void DocumentContainer::draw_list_marker(litehtml::uint_ptr hdc, const litehtml::list_marker& marker) {

}


litehtml::element::ptr	DocumentContainer::create_element( const char* tag_name,
													const litehtml::string_map& attributes,
													const std::shared_ptr<litehtml::document>& doc) {
	return nullptr;
}

void DocumentContainer::get_viewport(litehtml::position& viewport) const {
	Rect2 rect = this->get_global_rect();
	viewport.x = rect.position.x;
	viewport.y = rect.position.y;
	viewport.width = rect.size.width;
	viewport.height = rect.size.height;
}

void DocumentContainer::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_html", "html"), &DocumentContainer::set_html);
}


