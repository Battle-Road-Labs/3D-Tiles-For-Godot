#ifndef TILE_METADATA_H
#define TILE_METADATA_H

#include "CesiumGltf/PropertyTableView.h"
#include "CesiumGltf/PropertyTypeTraits.h"
#include "glm/detail/qualifier.hpp"
#include "godot_cpp/classes/ref_counted.hpp"
#include "godot_cpp/classes/wrapped.hpp"
#include "godot_cpp/variant/array.hpp"
#include "godot_cpp/variant/dictionary.hpp"
#include "godot_cpp/variant/string.hpp"
#include "godot_cpp/variant/variant.hpp"
#include "godot_cpp/variant/vector2.hpp"
#include "godot_cpp/variant/vector2i.hpp"
#include "godot_cpp/variant/vector3.hpp"
#include <cstddef>
#include <cstdint>
#include <string>
#include <type_traits>
#include <unordered_map>
#include <vector>
#if defined (CESIUM_GD_EXT)
using namespace godot;
#endif

enum class EPropertyType 
{
    Invalid = 0,
    Scalar,
    Vec2,
    Vec3,
    Vec4,
    Mat2,
    Mat3,
    Mat4,
    String,
    Boolean,
    Enum
};

/// @brief type of the underlying components of a type, i.e. if the type is a Vec3f, the components are 32-bit floats 
enum class EComponentType : int64_t 
{
    None = 0,
    Int8,
    Uint8,
    Int16,
    Uint16,
    Int32,
    Uint32,
    Int64,
    Uint64,
    Float32,
    Float64
};

class CesiumPropertyInfo : public RefCounted {
	GDCLASS(CesiumPropertyInfo, RefCounted)
	
protected:
	static void _bind_methods();

public:
	EPropertyType propertyType;
	EComponentType componentType;
	bool isArray;
	Variant data;
};

// using CesiumPropertyTable_t = std::unordered_map<std::string, CesiumPropertyInfo>;
using CesiumPropertyTable_t = Dictionary;

class TileMetadata  {

public:

	void init(size_t tableCount);

	void add_table(const CesiumGltf::PropertyTableView& tableView);

	const Dictionary& get_table(int32_t index) const;

	int32_t get_table_count() const;

private:

	template<class T>
	EComponentType get_component_type(const T& nativeType) {
	    using CurrentComponent_t = class T::value_type;
		if constexpr (CesiumGltf::IsMetadataInteger<CurrentComponent_t>::value) {
			if constexpr (std::is_unsigned_v<CurrentComponent_t>) {
				return EComponentType::Uint32;
			}
			return EComponentType::Int32;
		}
		if constexpr (std::is_same_v<CurrentComponent_t, float>) {
			return EComponentType::Float32;
		}
		if constexpr (std::is_same_v<CurrentComponent_t, double>) {
			return EComponentType::Float64;
		}
		return EComponentType::None;
	}

	
	template<class T>
	CesiumPropertyInfo make_vector_type(const T& nativeType) {
		CesiumPropertyInfo result;
		result.componentType = this->get_component_type(nativeType);
		
		if constexpr (T::length() == 2) {
        	result.propertyType = EPropertyType::Vec2;
			switch (result.componentType) {
			default:
				result.propertyType = EPropertyType::Invalid;
				ERR_PRINT("Metadata parsing error, Invalid component type index for Vector2");
				return result;
	        case EComponentType::Uint32:
	        	// TODO: Reconcile with glm
            case EComponentType::Int32:
            	result.data = Vector2i(nativeType[0], nativeType[1]);
            	break;
            case EComponentType::Float64:
            	#ifndef REAL_T_IS_DOUBLE
				WARN_PRINT("Metadata narrowing conversion from Vector2(double) to Vector2(float), use a custom double precision build to avoid this warning");
            	#endif
            case EComponentType::Float32:
        		result.data = Vector2(nativeType[0], nativeType[1]);
	              break;
	        }
	        return result;
		}
		if (T::length() == 3) {
			result.propertyType = EPropertyType::Vec3;
			switch (result.componentType) {
			default:
				result.propertyType = EPropertyType::Invalid;
				ERR_PRINT("Metadata parsing error, Invalid component type index for Vector3");
				return result;
	        case EComponentType::Uint32:
	        	// TODO: Reconcile with glm
            case EComponentType::Int32:
            	result.data = Vector3i(nativeType[0], nativeType[1], nativeType[2]);
            	break;
            case EComponentType::Float64:
            	#ifndef REAL_T_IS_DOUBLE
				WARN_PRINT("Metadata narrowing conversion from Vector3(double) to Vector3(float), use a custom double precision build to avoid this warning");
            	#endif
            case EComponentType::Float32:
            	result.data = Vector3(nativeType[0], nativeType[1], nativeType[2]);
	              break;
			}
			return result;
		}
		if (T::length() == 4) {
			result.propertyType = EPropertyType::Vec4;
			switch (result.componentType) {
			default:
				result.propertyType = EPropertyType::Invalid;
				ERR_PRINT("Metadata parsing error, Invalid component type index for Vector4");
				return result;
	        case EComponentType::Uint32:
	        	// TODO: Reconcile with glm
            case EComponentType::Int32:
            	result.data = Vector4i(nativeType[0], nativeType[1], nativeType[2], nativeType[3]);
            	break;
            case EComponentType::Float64:
            	#ifndef REAL_T_IS_DOUBLE
				WARN_PRINT("Metadata narrowing conversion from Vector4(double) to Vector4(float), use a custom double precision build to avoid this warning");
            	#endif
            case EComponentType::Float32:
            	result.data = Vector4(nativeType[0], nativeType[1], nativeType[2], nativeType[3]);
	              break;
			}
			return result;
		}
		result.propertyType = EPropertyType::Invalid;
		result.data = nullptr;
		return result;
	}

	template<class T>
	CesiumPropertyInfo make_array_type(const T& nativeValue) {
		CesiumPropertyInfo result;
		result.isArray = true;
		Array data = Array();
		data.resize(nativeValue.size());

		for (size_t i = 0; i < nativeValue.size(); i++) {
			CesiumPropertyInfo internalValue = this->make_metadata_value(nativeValue[i]);
			data[i] = internalValue.data;
			result.propertyType = internalValue.propertyType; 
			result.componentType = internalValue.componentType;
		}
		
		result.data = data;
		return result;
	}

	template<class T>
	Ref<CesiumPropertyInfo> make_metadata_value(const T& nativeValue) {
		CesiumPropertyInfo result;
		if constexpr (CesiumGltf::IsMetadataArray<T>::value) {
			// TODO: Make an array here
			result = this->make_array_type(nativeValue);
		}
		
		if constexpr (CesiumGltf::IsMetadataVecN<T>::value) {
			constexpr glm::length_t length = T::length();
			result = this->make_vector_type(nativeValue);
		}
		if constexpr (CesiumGltf::IsMetadataString<T>::value) {
			result.propertyType = EPropertyType::String;
			result.data = std::string(nativeValue.data(), nativeValue.size()).c_str();
		}

		if constexpr(CesiumGltf::IsMetadataBoolean<T>::value || CesiumGltf::IsMetadataScalar<T>::value) {
			result.propertyType = EPropertyType::Boolean;
			result.data = nativeValue;
		}
		
		return &result;
	}
	
	std::vector<CesiumPropertyTable_t> m_tables;

	Dictionary m_accesibleRepresentation;
	
};


#endif
