#ifndef TILE_METADATA_H
#define TILE_METADATA_H

#include "CesiumPropertyInfo.h"
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
		CesiumPropertyInfo result{
			.isArray = true
		};
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
		CesiumPropertyInfo* result = memnew(CesiumPropertyInfo);
		//CesiumGltf::PropertyType nativeType = nativeValue.propertyType();

		using OptionalType = std::decay_t<decltype(nativeValue.get(0))>;
		using ValueType = typename OptionalType::value_type;

		int64_t rowCount = nativeValue.size();
		if constexpr (CesiumGltf::IsMetadataString<ValueType>::value) {
			
			std::string result_str;
			std::string str_temp = "";
			std::string last_str = "";
			for (int64_t idx = 0; idx < rowCount; idx++) {
				auto data_value = nativeValue.get(idx);
				
				if (!data_value) continue;
				if (!data_value.has_value()) continue;
				
				std::string_view sv = *data_value;
				auto first_value = sv.data();
				str_temp = std::string(sv.data(), sv.size());
				if (str_temp == last_str) continue;
				if (str_temp.empty()) continue;

				result_str += std::string(sv.data(), sv.size());
				last_str = std::string(sv.data(), sv.size());
			}
			//printf(" string_value: %s\n", result_str.c_str());
			
			result->componentType = EComponentType::None;
			result->propertyType = EPropertyType::String;
			result->data = godot::String(result_str.c_str());
		}
		else if constexpr (CesiumGltf::IsMetadataScalar<ValueType>::value) {
			if (rowCount > 0) {
				auto data_value = nativeValue.get(0);
				if (data_value && data_value.has_value()) {
					if constexpr (std::is_same_v<ValueType, float>) {
						float num_data = static_cast<float>(*data_value);
						result->data = godot::Variant(num_data);
						result->componentType = EComponentType::Float32;
					}
					else if constexpr (std::is_same_v<ValueType, double>) {
						double num_data = static_cast<double>(*data_value);
						result->data = godot::Variant(num_data);
						result->componentType = EComponentType::Float64;
					}
					else if constexpr (std::is_same_v<ValueType, int>) {
						int num_data = static_cast<int>(*data_value);
						result->data = godot::Variant(num_data);
						result->componentType = EComponentType::Int32;
					}

					result->propertyType = EPropertyType::Scalar;
					//printf(" scalar_value: %s\n", std::string(godot::String(result->data).utf8().get_data()).c_str());
				}
			}
		}
		else if constexpr (CesiumGltf::IsMetadataArray<ValueType>::value) {
			// TODO: Make an array here
			//*result = *this->make_array_type(nativeValue);
		}
		else if constexpr (CesiumGltf::IsMetadataVecN<ValueType>::value) {
			//constexpr glm::length_t length = T::length();
			//*result = this->make_vector_type(nativeValue);
		}
		else if constexpr (CesiumGltf::IsMetadataBoolean<ValueType>::value) {
			result->propertyType = EPropertyType::Boolean;
			//result->data = nativeValue;
		}

		return result;
	}

	std::vector<CesiumPropertyTable_t> m_tables;

	Dictionary m_empty;
	
};


#endif
