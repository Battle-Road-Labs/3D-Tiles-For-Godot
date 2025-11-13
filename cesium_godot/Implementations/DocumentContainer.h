#ifndef DOCUMENT_CONTAINER_H
#define DOCUMENT_CONTAINER_H


#include "executable_control.hpp"
#include "Utils/CurlHttpClient.h"
#if defined (CESIUM_GD_EXT)
#include "godot_cpp/classes/control.hpp"
#include "godot_cpp/classes/image_texture.hpp"
#include "godot_cpp/classes/wrapped.hpp"
using namespace godot;
#else
#include "scene/gui/control.h"
#include "scene/resources/image_texture.h"
#endif
#include "litehtml.h"
#include "litehtml/document.h"
#include <cstdint>
#include <unordered_map>


#ifndef _EXE_CONTROL
#define _EXE_CONTROL

class ExecutableControl : public Control {
private:
	void operator=(const ExecutableControl &p_rval) {}
	friend class ::ClassDB;

public:
	typedef ExecutableControl self_type;
	static constexpr bool _class_is_enabled = !bool(false) && Control ::_class_is_enabled;
	virtual String get_class() const override {
		if (_get_extension()) {
			return _get_extension()->class_name.operator String();
		}
		return String("ExecutableControl");
	}
	virtual const StringName *_get_class_namev() const override {
		static StringName _class_name_static;
		if (!_class_name_static) {
			StringName ::assign_static_unique_class_name(&_class_name_static, "ExecutableControl");
		}
		return &_class_name_static;
	}
	static __forceinline void *get_class_ptr_static() {
		static int ptr;
		return &ptr;
	}
	static __forceinline String get_class_static() { return String("ExecutableControl"); }
	static __forceinline String get_parent_class_static() { return Control ::get_class_static(); }
	static void get_inheritance_list_static(List<String> *p_inheritance_list) {
		Control ::get_inheritance_list_static(p_inheritance_list);
		p_inheritance_list->push_back(String("ExecutableControl"));
	}
	virtual bool is_class(const String &p_class) const override {
		if (_get_extension() && _get_extension()->is_class(p_class)) {
			return true;
		}
		return (p_class == ("ExecutableControl")) ? true : Control ::is_class(p_class);
	}
	virtual bool is_class_ptr(void *p_ptr) const override { return (p_ptr == get_class_ptr_static()) ? true : Control ::is_class_ptr(p_ptr); }
	static void get_valid_parents_static(List<String> *p_parents) {
		if (ExecutableControl ::_get_valid_parents_static != Control ::_get_valid_parents_static) {
			ExecutableControl ::_get_valid_parents_static(p_parents);
		}
		Control ::get_valid_parents_static(p_parents);
	}

protected:
	__forceinline static void (*_get_bind_methods())() { return &ExecutableControl ::_bind_methods; }
	__forceinline static void (*_get_bind_compatibility_methods())() { return &ExecutableControl ::_bind_compatibility_methods; }

public:
	static void initialize_class() {
		static bool initialized = false;
		if (initialized) {
			return;
		}
		Control ::initialize_class();
		::ClassDB ::_add_class<ExecutableControl>();
		if (ExecutableControl ::_get_bind_methods() != Control ::_get_bind_methods()) {
			_bind_methods();
		}
		if (ExecutableControl ::_get_bind_compatibility_methods() != Control ::_get_bind_compatibility_methods()) {
			_bind_compatibility_methods();
		}
		initialized = true;
	}

protected:
	virtual void _initialize_classv() override { initialize_class(); }
	__forceinline bool (Object ::*_get_get() const)(const StringName &p_name, Variant &) const { return (bool(Object ::*)(const StringName &, Variant &) const) & ExecutableControl ::_get; }
	virtual bool _getv(const StringName &p_name, Variant &r_ret) const override {
		if (ExecutableControl ::_get_get() != Control ::_get_get()) {
			if (_get(p_name, r_ret)) {
				return true;
			}
		}
		return Control ::_getv(p_name, r_ret);
	}
	__forceinline bool (Object ::*_get_set() const)(const StringName &p_name, const Variant &p_property) { return (bool(Object ::*)(const StringName &, const Variant &)) & ExecutableControl ::_set; }
	virtual bool _setv(const StringName &p_name, const Variant &p_property) override {
		if (Control ::_setv(p_name, p_property)) {
			return true;
		}
		if (ExecutableControl ::_get_set() != Control ::_get_set()) {
			return _set(p_name, p_property);
		}
		return false;
	}
	__forceinline void (Object ::*_get_get_property_list() const)(List<PropertyInfo> *p_list) const { return (void(Object ::*)(List<PropertyInfo> *) const) & ExecutableControl ::_get_property_list; }
	virtual void _get_property_listv(List<PropertyInfo> *p_list, bool p_reversed) const override {
		if (!p_reversed) {
			Control ::_get_property_listv(p_list, p_reversed);
		}
		p_list->push_back(PropertyInfo(Variant ::NIL, get_class_static(), PROPERTY_HINT_NONE, get_class_static(), PROPERTY_USAGE_CATEGORY));
		::ClassDB ::get_property_list("ExecutableControl", p_list, true, this);
		if (ExecutableControl ::_get_get_property_list() != Control ::_get_get_property_list()) {
			_get_property_list(p_list);
		}
		if (p_reversed) {
			Control ::_get_property_listv(p_list, p_reversed);
		}
	}
	__forceinline void (Object ::*_get_validate_property() const)(PropertyInfo &p_property) const { return (void(Object ::*)(PropertyInfo &) const) & ExecutableControl ::_validate_property; }
	virtual void _validate_propertyv(PropertyInfo &p_property) const override {
		Control ::_validate_propertyv(p_property);
		if (ExecutableControl ::_get_validate_property() != Control ::_get_validate_property()) {
			_validate_property(p_property);
		}
	}
	__forceinline bool (Object ::*_get_property_can_revert() const)(const StringName &p_name) const { return (bool(Object ::*)(const StringName &) const) & ExecutableControl ::_property_can_revert; }
	virtual bool _property_can_revertv(const StringName &p_name) const override {
		if (ExecutableControl ::_get_property_can_revert() != Control ::_get_property_can_revert()) {
			if (_property_can_revert(p_name)) {
				return true;
			}
		}
		return Control ::_property_can_revertv(p_name);
	}
	__forceinline bool (Object ::*_get_property_get_revert() const)(const StringName &p_name, Variant &) const { return (bool(Object ::*)(const StringName &, Variant &) const) & ExecutableControl ::_property_get_revert; }
	virtual bool _property_get_revertv(const StringName &p_name, Variant &r_ret) const override {
		if (ExecutableControl ::_get_property_get_revert() != Control ::_get_property_get_revert()) {
			if (_property_get_revert(p_name, r_ret)) {
				return true;
			}
		}
		return Control ::_property_get_revertv(p_name, r_ret);
	}
	__forceinline void (Object ::*_get_notification() const)(int) { return (void(Object ::*)(int)) & ExecutableControl ::_notification; }
	virtual void _notificationv(int p_notification, bool p_reversed) override {
		if (!p_reversed) {
			Control ::_notificationv(p_notification, p_reversed);
		}
		if (ExecutableControl ::_get_notification() != Control ::_get_notification()) {
			_notification(p_notification);
		}
		if (p_reversed) {
			Control ::_notificationv(p_notification, p_reversed);
		}
	}

private:
public:
	virtual void _enter_tree() {}
	virtual void _exit_tree() {}
	virtual void _ready() {}
	virtual void _draw() {}
	virtual void _process(real_t delta) {}

protected:
	using Control ::_notification;
	void _notification(int p_what) {
		switch (p_what) {
			case NOTIFICATION_READY:
				this->_ready();
				break;
			case NOTIFICATION_PROCESS:
				this->_process(p_what);
				break;
			case NOTIFICATION_ENTER_TREE:
				this->_enter_tree();
				break;
			case NOTIFICATION_EXIT_TREE:
				this->_exit_tree();
				break;
			case CanvasItem ::NOTIFICATION_DRAW:
				this->_draw();
				break;
			default:
				break;
		}
	}
	static void _bind_methods() {}
};
#endif

class DocumentContainer final : public ExecutableControl, public litehtml::document_container {
	GDCLASS(DocumentContainer, ExecutableControl)
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
	litehtml::element::ptr	create_element(const char* tag_name,
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
	CurlHttpClient<2> m_httpClient;
	// I know this is "backwards" if you think abt this in OOP terms, but I beg you to think of this class as just functional implementations
	litehtml::document::ptr m_document = nullptr;

protected:
	static void _bind_methods();
	
};

#endif
