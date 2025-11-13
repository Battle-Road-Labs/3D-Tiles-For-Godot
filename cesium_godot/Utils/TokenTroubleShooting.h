#ifndef TOKEN_TROBLESHOOTING_H
#define TOKEN_TROBLESHOOTING_H

#include "Utils/CurlHttpClient.h"
#include <cstdint>
#include <unordered_map>
#include "executable_node.hpp"
#if defined(CESIUM_GD_EXT)
#include "godot_cpp/variant/variant.hpp"
#include "godot_cpp/classes/node.hpp"
using namespace godot;
#else
#include "scene/main/node.h"
#endif



#ifndef _EXE_NODE
#define _EXE_NODE

class ExecutableNode : public Node {
private:
	void operator=(const ExecutableNode &p_rval) {}
	friend class ::ClassDB;

public:
	typedef ExecutableNode self_type;
	static constexpr bool _class_is_enabled = !bool(false) && Node ::_class_is_enabled;
	virtual String get_class() const override {
		if (_get_extension()) {
			return _get_extension()->class_name.operator String();
		}
		return String("ExecutableNode");
	}
	virtual const StringName *_get_class_namev() const override {
		static StringName _class_name_static;
		if (!_class_name_static) {
			StringName ::assign_static_unique_class_name(&_class_name_static, "ExecutableNode");
		}
		return &_class_name_static;
	}
	static __forceinline void *get_class_ptr_static() {
		static int ptr;
		return &ptr;
	}
	static __forceinline String get_class_static() { return String("ExecutableNode"); }
	static __forceinline String get_parent_class_static() { return Node ::get_class_static(); }
	static void get_inheritance_list_static(List<String> *p_inheritance_list) {
		Node ::get_inheritance_list_static(p_inheritance_list);
		p_inheritance_list->push_back(String("ExecutableNode"));
	}
	virtual bool is_class(const String &p_class) const override {
		if (_get_extension() && _get_extension()->is_class(p_class)) {
			return true;
		}
		return (p_class == ("ExecutableNode")) ? true : Node ::is_class(p_class);
	}
	virtual bool is_class_ptr(void *p_ptr) const override { return (p_ptr == get_class_ptr_static()) ? true : Node ::is_class_ptr(p_ptr); }
	static void get_valid_parents_static(List<String> *p_parents) {
		if (ExecutableNode ::_get_valid_parents_static != Node ::_get_valid_parents_static) {
			ExecutableNode ::_get_valid_parents_static(p_parents);
		}
		Node ::get_valid_parents_static(p_parents);
	}

protected:
	__forceinline static void (*_get_bind_methods())() { return &ExecutableNode ::_bind_methods; }
	__forceinline static void (*_get_bind_compatibility_methods())() { return &ExecutableNode ::_bind_compatibility_methods; }

public:
	static void initialize_class() {
		static bool initialized = false;
		if (initialized) {
			return;
		}
		Node ::initialize_class();
		::ClassDB ::_add_class<ExecutableNode>();
		if (ExecutableNode ::_get_bind_methods() != Node ::_get_bind_methods()) {
			_bind_methods();
		}
		if (ExecutableNode ::_get_bind_compatibility_methods() != Node ::_get_bind_compatibility_methods()) {
			_bind_compatibility_methods();
		}
		initialized = true;
	}

protected:
	virtual void _initialize_classv() override { initialize_class(); }
	__forceinline bool (Object ::*_get_get() const)(const StringName &p_name, Variant &) const { return (bool(Object ::*)(const StringName &, Variant &) const) & ExecutableNode ::_get; }
	virtual bool _getv(const StringName &p_name, Variant &r_ret) const override {
		if (ExecutableNode ::_get_get() != Node ::_get_get()) {
			if (_get(p_name, r_ret)) {
				return true;
			}
		}
		return Node ::_getv(p_name, r_ret);
	}
	__forceinline bool (Object ::*_get_set() const)(const StringName &p_name, const Variant &p_property) { return (bool(Object ::*)(const StringName &, const Variant &)) & ExecutableNode ::_set; }
	virtual bool _setv(const StringName &p_name, const Variant &p_property) override {
		if (Node ::_setv(p_name, p_property)) {
			return true;
		}
		if (ExecutableNode ::_get_set() != Node ::_get_set()) {
			return _set(p_name, p_property);
		}
		return false;
	}
	__forceinline void (Object ::*_get_get_property_list() const)(List<PropertyInfo> *p_list) const { return (void(Object ::*)(List<PropertyInfo> *) const) & ExecutableNode ::_get_property_list; }
	virtual void _get_property_listv(List<PropertyInfo> *p_list, bool p_reversed) const override {
		if (!p_reversed) {
			Node ::_get_property_listv(p_list, p_reversed);
		}
		p_list->push_back(PropertyInfo(Variant ::NIL, get_class_static(), PROPERTY_HINT_NONE, get_class_static(), PROPERTY_USAGE_CATEGORY));
		::ClassDB ::get_property_list("ExecutableNode", p_list, true, this);
		if (ExecutableNode ::_get_get_property_list() != Node ::_get_get_property_list()) {
			_get_property_list(p_list);
		}
		if (p_reversed) {
			Node ::_get_property_listv(p_list, p_reversed);
		}
	}
	__forceinline void (Object ::*_get_validate_property() const)(PropertyInfo &p_property) const { return (void(Object ::*)(PropertyInfo &) const) & ExecutableNode ::_validate_property; }
	virtual void _validate_propertyv(PropertyInfo &p_property) const override {
		Node ::_validate_propertyv(p_property);
		if (ExecutableNode ::_get_validate_property() != Node ::_get_validate_property()) {
			_validate_property(p_property);
		}
	}
	__forceinline bool (Object ::*_get_property_can_revert() const)(const StringName &p_name) const { return (bool(Object ::*)(const StringName &) const) & ExecutableNode ::_property_can_revert; }
	virtual bool _property_can_revertv(const StringName &p_name) const override {
		if (ExecutableNode ::_get_property_can_revert() != Node ::_get_property_can_revert()) {
			if (_property_can_revert(p_name)) {
				return true;
			}
		}
		return Node ::_property_can_revertv(p_name);
	}
	__forceinline bool (Object ::*_get_property_get_revert() const)(const StringName &p_name, Variant &) const { return (bool(Object ::*)(const StringName &, Variant &) const) & ExecutableNode ::_property_get_revert; }
	virtual bool _property_get_revertv(const StringName &p_name, Variant &r_ret) const override {
		if (ExecutableNode ::_get_property_get_revert() != Node ::_get_property_get_revert()) {
			if (_property_get_revert(p_name, r_ret)) {
				return true;
			}
		}
		return Node ::_property_get_revertv(p_name, r_ret);
	}
	__forceinline void (Object ::*_get_notification() const)(int) { return (void(Object ::*)(int)) & ExecutableNode ::_notification; }
	virtual void _notificationv(int p_notification, bool p_reversed) override {
		if (!p_reversed) {
			Node ::_notificationv(p_notification, p_reversed);
		}
		if (ExecutableNode ::_get_notification() != Node ::_get_notification()) {
			_notification(p_notification);
		}
		if (p_reversed) {
			Node ::_notificationv(p_notification, p_reversed);
		}
	}

private:
public:
	virtual void _enter_tree() {}
	virtual void _exit_tree() {}
	virtual void _ready() {}
	virtual void _process(real_t delta) {}

protected:
	using Node ::_notification;
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

class TokenTroubleshooting : public ExecutableNode {
  GDCLASS(TokenTroubleshooting, ExecutableNode)
  public:
    void is_valid_token(const String& token);

    void on_token_validity_check(const String& token, bool isValid, const std::unordered_map<std::string, int32_t>& p_data);
    
    void set_data(const Variant &p_data);

    int32_t get_asset_id_by_name(const String &name) const {
      return this->m_lastAssetLists.at(name.utf8().get_data());
    }

    void _exit_tree() override;
    
  protected:
    static void _bind_methods();

  private:
    CurlHttpClient<1> m_httpClient{};
    Variant m_tokenData;
    std::unordered_map<std::string, int32_t> m_lastAssetLists;   
};

#endif
