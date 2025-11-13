#ifndef GEOREFERENCED_NODE_H
#define GEOREFERENCED_NODE_H

#include "executable_node.hpp"
#if defined (CESIUM_GD_EXT)
#include "godot_cpp/classes/mesh_instance3d.hpp"
#include "godot_cpp/variant/vector3.hpp"
using namespace godot;
#else
#include "scene/3d/mesh_instance_3d.h"
#endif
#include "glm/ext/vector_double3.hpp"
class CesiumGeoreference;

class Cesium3DTileset;

// Not a fan of OOP, but, oh well


#ifndef _EXE_MESH_INSTANCE3D
#define _EXE_MESH_INSTANCE3D

class ExecutableMeshInstance3D : public MeshInstance3D {
private:
	void operator=(const ExecutableMeshInstance3D &p_rval) {}
	friend class ::ClassDB;

public:
	typedef ExecutableMeshInstance3D self_type;
	static constexpr bool _class_is_enabled = !bool(false) && MeshInstance3D ::_class_is_enabled;
	virtual String get_class() const override {
		if (_get_extension()) {
			return _get_extension()->class_name.operator String();
		}
		return String("ExecutableMeshInstance3D");
	}
	virtual const StringName *_get_class_namev() const override {
		static StringName _class_name_static;
		if (!_class_name_static) {
			StringName ::assign_static_unique_class_name(&_class_name_static, "ExecutableMeshInstance3D");
		}
		return &_class_name_static;
	}
	static __forceinline void *get_class_ptr_static() {
		static int ptr;
		return &ptr;
	}
	static __forceinline String get_class_static() { return String("ExecutableMeshInstance3D"); }
	static __forceinline String get_parent_class_static() { return MeshInstance3D ::get_class_static(); }
	static void get_inheritance_list_static(List<String> *p_inheritance_list) {
		MeshInstance3D ::get_inheritance_list_static(p_inheritance_list);
		p_inheritance_list->push_back(String("ExecutableMeshInstance3D"));
	}
	virtual bool is_class(const String &p_class) const override {
		if (_get_extension() && _get_extension()->is_class(p_class)) {
			return true;
		}
		return (p_class == ("ExecutableMeshInstance3D")) ? true : MeshInstance3D ::is_class(p_class);
	}
	virtual bool is_class_ptr(void *p_ptr) const override { return (p_ptr == get_class_ptr_static()) ? true : MeshInstance3D ::is_class_ptr(p_ptr); }
	static void get_valid_parents_static(List<String> *p_parents) {
		if (ExecutableMeshInstance3D ::_get_valid_parents_static != MeshInstance3D ::_get_valid_parents_static) {
			ExecutableMeshInstance3D ::_get_valid_parents_static(p_parents);
		}
		MeshInstance3D ::get_valid_parents_static(p_parents);
	}

protected:
	__forceinline static void (*_get_bind_methods())() { return &ExecutableMeshInstance3D ::_bind_methods; }
	__forceinline static void (*_get_bind_compatibility_methods())() { return &ExecutableMeshInstance3D ::_bind_compatibility_methods; }

public:
	static void initialize_class() {
		static bool initialized = false;
		if (initialized) {
			return;
		}
		MeshInstance3D ::initialize_class();
		::ClassDB ::_add_class<ExecutableMeshInstance3D>();
		if (ExecutableMeshInstance3D ::_get_bind_methods() != MeshInstance3D ::_get_bind_methods()) {
			_bind_methods();
		}
		if (ExecutableMeshInstance3D ::_get_bind_compatibility_methods() != MeshInstance3D ::_get_bind_compatibility_methods()) {
			_bind_compatibility_methods();
		}
		initialized = true;
	}

protected:
	virtual void _initialize_classv() override { initialize_class(); }
	__forceinline bool (Object ::*_get_get() const)(const StringName &p_name, Variant &) const { return (bool(Object ::*)(const StringName &, Variant &) const) & ExecutableMeshInstance3D ::_get; }
	virtual bool _getv(const StringName &p_name, Variant &r_ret) const override {
		if (ExecutableMeshInstance3D ::_get_get() != MeshInstance3D ::_get_get()) {
			if (_get(p_name, r_ret)) {
				return true;
			}
		}
		return MeshInstance3D ::_getv(p_name, r_ret);
	}
	__forceinline bool (Object ::*_get_set() const)(const StringName &p_name, const Variant &p_property) { return (bool(Object ::*)(const StringName &, const Variant &)) & ExecutableMeshInstance3D ::_set; }
	virtual bool _setv(const StringName &p_name, const Variant &p_property) override {
		if (MeshInstance3D ::_setv(p_name, p_property)) {
			return true;
		}
		if (ExecutableMeshInstance3D ::_get_set() != MeshInstance3D ::_get_set()) {
			return _set(p_name, p_property);
		}
		return false;
	}
	__forceinline void (Object ::*_get_get_property_list() const)(List<PropertyInfo> *p_list) const { return (void(Object ::*)(List<PropertyInfo> *) const) & ExecutableMeshInstance3D ::_get_property_list; }
	virtual void _get_property_listv(List<PropertyInfo> *p_list, bool p_reversed) const override {
		if (!p_reversed) {
			MeshInstance3D ::_get_property_listv(p_list, p_reversed);
		}
		p_list->push_back(PropertyInfo(Variant ::NIL, get_class_static(), PROPERTY_HINT_NONE, get_class_static(), PROPERTY_USAGE_CATEGORY));
		::ClassDB ::get_property_list("ExecutableMeshInstance3D", p_list, true, this);
		if (ExecutableMeshInstance3D ::_get_get_property_list() != MeshInstance3D ::_get_get_property_list()) {
			_get_property_list(p_list);
		}
		if (p_reversed) {
			MeshInstance3D ::_get_property_listv(p_list, p_reversed);
		}
	}
	__forceinline void (Object ::*_get_validate_property() const)(PropertyInfo &p_property) const { return (void(Object ::*)(PropertyInfo &) const) & ExecutableMeshInstance3D ::_validate_property; }
	virtual void _validate_propertyv(PropertyInfo &p_property) const override {
		MeshInstance3D ::_validate_propertyv(p_property);
		if (ExecutableMeshInstance3D ::_get_validate_property() != MeshInstance3D ::_get_validate_property()) {
			_validate_property(p_property);
		}
	}
	__forceinline bool (Object ::*_get_property_can_revert() const)(const StringName &p_name) const { return (bool(Object ::*)(const StringName &) const) & ExecutableMeshInstance3D ::_property_can_revert; }
	virtual bool _property_can_revertv(const StringName &p_name) const override {
		if (ExecutableMeshInstance3D ::_get_property_can_revert() != MeshInstance3D ::_get_property_can_revert()) {
			if (_property_can_revert(p_name)) {
				return true;
			}
		}
		return MeshInstance3D ::_property_can_revertv(p_name);
	}
	__forceinline bool (Object ::*_get_property_get_revert() const)(const StringName &p_name, Variant &) const { return (bool(Object ::*)(const StringName &, Variant &) const) & ExecutableMeshInstance3D ::_property_get_revert; }
	virtual bool _property_get_revertv(const StringName &p_name, Variant &r_ret) const override {
		if (ExecutableMeshInstance3D ::_get_property_get_revert() != MeshInstance3D ::_get_property_get_revert()) {
			if (_property_get_revert(p_name, r_ret)) {
				return true;
			}
		}
		return MeshInstance3D ::_property_get_revertv(p_name, r_ret);
	}
	__forceinline void (Object ::*_get_notification() const)(int) { return (void(Object ::*)(int)) & ExecutableMeshInstance3D ::_notification; }
	virtual void _notificationv(int p_notification, bool p_reversed) override {
		if (!p_reversed) {
			MeshInstance3D ::_notificationv(p_notification, p_reversed);
		}
		if (ExecutableMeshInstance3D ::_get_notification() != MeshInstance3D ::_get_notification()) {
			_notification(p_notification);
		}
		if (p_reversed) {
			MeshInstance3D ::_notificationv(p_notification, p_reversed);
		}
	}

private:
public:
	virtual void _enter_tree() {}
	virtual void _exit_tree() {}
	virtual void _ready() {}
	virtual void _process(real_t delta) {}

protected:
	using MeshInstance3D ::_notification;
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

class GeoreferencedMesh : public ExecutableMeshInstance3D {
	GDCLASS(GeoreferencedMesh, ExecutableMeshInstance3D)

public:
	void _ready() override;
	
	void apply_position_on_globe(const glm::dvec3& engineOrigin);
	
	const glm::dvec3& get_original_position() const;
	
	/// @brief Intended to be used by C++ only
	void set_original_position(const glm::dvec3& position);

	/// @brief Intended to be used by GDScript only
	void set_engine_position(const Vector3& position);

	/// @brief Intended to be used by GDScript only
	void set_ecef_position(const Vector3& position);

	Vector3 get_ecef_position() const;
	
	Vector3 get_engine_position() const;

	void set_tileset(Cesium3DTileset* tileset);
	
	Cesium3DTileset* get_tileset() const;
	
	void set_tileset_no_reparent(Cesium3DTileset* tileset);
	
	CesiumGeoreference* get_georeference() const;
	
	// void set_ecef_position(const Vector3& position);


protected:
	glm::dvec3 m_originalPosition;

	Cesium3DTileset* m_tileset = nullptr;

	static void _bind_methods();
};

#endif
