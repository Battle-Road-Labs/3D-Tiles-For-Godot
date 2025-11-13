#ifndef CESIUM_GD_CONFIG
#define CESIUM_GD_CONFIG

#include "executable_node.hpp"

#if defined(CESIUM_GD_EXT)
#include "godot_cpp/classes/node3d.hpp"
#include <godot_cpp/classes/resource.hpp>
using namespace godot;
#else
#include "scene/3d/node_3d.h"
#include "core/io/resource.h"
#endif

#ifndef _EXE_NODE3D
#define _EXE_NODE3D

class ExecutableNode3D : public Node3D {
private:
	void operator=(const ExecutableNode3D &p_rval) {}
	friend class ::ClassDB;

public:
	typedef ExecutableNode3D self_type;
	static constexpr bool _class_is_enabled = !bool(false) && Node3D ::_class_is_enabled;
	virtual String get_class() const override {
		if (_get_extension()) {
			return _get_extension()->class_name.operator String();
		}
		return String("ExecutableNode3D");
	}
	virtual const StringName *_get_class_namev() const override {
		static StringName _class_name_static;
		if (!_class_name_static) {
			StringName ::assign_static_unique_class_name(&_class_name_static, "ExecutableNode3D");
		}
		return &_class_name_static;
	}
	static __forceinline void *get_class_ptr_static() {
		static int ptr;
		return &ptr;
	}
	static __forceinline String get_class_static() { return String("ExecutableNode3D"); }
	static __forceinline String get_parent_class_static() { return Node3D ::get_class_static(); }
	static void get_inheritance_list_static(List<String> *p_inheritance_list) {
		Node3D ::get_inheritance_list_static(p_inheritance_list);
		p_inheritance_list->push_back(String("ExecutableNode3D"));
	}
	virtual bool is_class(const String &p_class) const override {
		if (_get_extension() && _get_extension()->is_class(p_class)) {
			return true;
		}
		return (p_class == ("ExecutableNode3D")) ? true : Node3D ::is_class(p_class);
	}
	virtual bool is_class_ptr(void *p_ptr) const override { return (p_ptr == get_class_ptr_static()) ? true : Node3D ::is_class_ptr(p_ptr); }
	static void get_valid_parents_static(List<String> *p_parents) {
		if (ExecutableNode3D ::_get_valid_parents_static != Node3D ::_get_valid_parents_static) {
			ExecutableNode3D ::_get_valid_parents_static(p_parents);
		}
		Node3D ::get_valid_parents_static(p_parents);
	}

protected:
	__forceinline static void (*_get_bind_methods())() { return &ExecutableNode3D ::_bind_methods; }
	__forceinline static void (*_get_bind_compatibility_methods())() { return &ExecutableNode3D ::_bind_compatibility_methods; }

public:
	static void initialize_class() {
		static bool initialized = false;
		if (initialized) {
			return;
		}
		Node3D ::initialize_class();
		::ClassDB ::_add_class<ExecutableNode3D>();
		if (ExecutableNode3D ::_get_bind_methods() != Node3D ::_get_bind_methods()) {
			_bind_methods();
		}
		if (ExecutableNode3D ::_get_bind_compatibility_methods() != Node3D ::_get_bind_compatibility_methods()) {
			_bind_compatibility_methods();
		}
		initialized = true;
	}

protected:
	virtual void _initialize_classv() override { initialize_class(); }
	__forceinline bool (Object ::*_get_get() const)(const StringName &p_name, Variant &) const { return (bool(Object ::*)(const StringName &, Variant &) const) & ExecutableNode3D ::_get; }
	virtual bool _getv(const StringName &p_name, Variant &r_ret) const override {
		if (ExecutableNode3D ::_get_get() != Node3D ::_get_get()) {
			if (_get(p_name, r_ret)) {
				return true;
			}
		}
		return Node3D ::_getv(p_name, r_ret);
	}
	__forceinline bool (Object ::*_get_set() const)(const StringName &p_name, const Variant &p_property) { return (bool(Object ::*)(const StringName &, const Variant &)) & ExecutableNode3D ::_set; }
	virtual bool _setv(const StringName &p_name, const Variant &p_property) override {
		if (Node3D ::_setv(p_name, p_property)) {
			return true;
		}
		if (ExecutableNode3D ::_get_set() != Node3D ::_get_set()) {
			return _set(p_name, p_property);
		}
		return false;
	}
	__forceinline void (Object ::*_get_get_property_list() const)(List<PropertyInfo> *p_list) const { return (void(Object ::*)(List<PropertyInfo> *) const) & ExecutableNode3D ::_get_property_list; }
	virtual void _get_property_listv(List<PropertyInfo> *p_list, bool p_reversed) const override {
		if (!p_reversed) {
			Node3D ::_get_property_listv(p_list, p_reversed);
		}
		p_list->push_back(PropertyInfo(Variant ::NIL, get_class_static(), PROPERTY_HINT_NONE, get_class_static(), PROPERTY_USAGE_CATEGORY));
		::ClassDB ::get_property_list("ExecutableNode3D", p_list, true, this);
		if (ExecutableNode3D ::_get_get_property_list() != Node3D ::_get_get_property_list()) {
			_get_property_list(p_list);
		}
		if (p_reversed) {
			Node3D ::_get_property_listv(p_list, p_reversed);
		}
	}
	__forceinline void (Object ::*_get_validate_property() const)(PropertyInfo &p_property) const { return (void(Object ::*)(PropertyInfo &) const) & ExecutableNode3D ::_validate_property; }
	virtual void _validate_propertyv(PropertyInfo &p_property) const override {
		Node3D ::_validate_propertyv(p_property);
		if (ExecutableNode3D ::_get_validate_property() != Node3D ::_get_validate_property()) {
			_validate_property(p_property);
		}
	}
	__forceinline bool (Object ::*_get_property_can_revert() const)(const StringName &p_name) const { return (bool(Object ::*)(const StringName &) const) & ExecutableNode3D ::_property_can_revert; }
	virtual bool _property_can_revertv(const StringName &p_name) const override {
		if (ExecutableNode3D ::_get_property_can_revert() != Node3D ::_get_property_can_revert()) {
			if (_property_can_revert(p_name)) {
				return true;
			}
		}
		return Node3D ::_property_can_revertv(p_name);
	}
	__forceinline bool (Object ::*_get_property_get_revert() const)(const StringName &p_name, Variant &) const { return (bool(Object ::*)(const StringName &, Variant &) const) & ExecutableNode3D ::_property_get_revert; }
	virtual bool _property_get_revertv(const StringName &p_name, Variant &r_ret) const override {
		if (ExecutableNode3D ::_get_property_get_revert() != Node3D ::_get_property_get_revert()) {
			if (_property_get_revert(p_name, r_ret)) {
				return true;
			}
		}
		return Node3D ::_property_get_revertv(p_name, r_ret);
	}
	__forceinline void (Object ::*_get_notification() const)(int) { return (void(Object ::*)(int)) & ExecutableNode3D ::_notification; }
	virtual void _notificationv(int p_notification, bool p_reversed) override {
		if (!p_reversed) {
			Node3D ::_notificationv(p_notification, p_reversed);
		}
		if (ExecutableNode3D ::_get_notification() != Node3D ::_get_notification()) {
			_notification(p_notification);
		}
		if (p_reversed) {
			Node3D ::_notificationv(p_notification, p_reversed);
		}
	}

private:
public:
	virtual void _enter_tree() {}
	virtual void _exit_tree() {}
	virtual void _ready() {}
	virtual void _process(real_t delta) {}

protected:
	using Node3D ::_notification;
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
			default:
				break;
		}
	}
	static void _bind_methods() {}
};
#endif

/// @brief Configuration resource for Cesium's server components
class CesiumGDConfig : public ExecutableNode3D {
	GDCLASS(CesiumGDConfig, ExecutableNode3D)
public:
	static constexpr std::string_view DEFAULT_ION_API_URL = "https://api.cesium.com/";
	static constexpr std::string_view DEFAULT_SERVER_URL = "https://ion.cesium.com";
	static constexpr int32_t DEFAULT_APPLICATION_ID = 891;

	CesiumGDConfig() = default;

	void set_access_token(const String& accessToken);

	const String& get_access_token() const;

	static CesiumGDConfig* get_singleton(Node* baseNode);

	static void clear_session();
		
	void _enter_tree() override;
	
private:
	Error create_cache_session_file();
	
	String m_accessToken = "";
	static inline CesiumGDConfig* s_instance = nullptr;
	
protected:
	static void _bind_methods();
};

#endif // CESIUM_GD_CONFIG
