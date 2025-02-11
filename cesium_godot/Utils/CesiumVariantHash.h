#ifndef CESIUM_VARIANT_HASH_H
#define CESIUM_VARIANT_HASH_H
#include <type_traits>
#include "CesiumGeometry/OctreeTileID.h"


struct CesiumVariantHash {
	template<class T>
	size_t operator()(const T& value) const {
		return std::hash<T>{}(value);
	}
};

namespace std {

	/**
	 * @brief A hash function for {@link CesiumGeometry::OctreeTileID} objects.
	 */
	template <> struct hash<CesiumGeometry::OctreeTileID> {

		/**
		 * @brief A specialization of the `std::hash` template for
		 * {@link CesiumGeometry::OctreeTileID} objects.
		 */
		size_t operator()(const CesiumGeometry::OctreeTileID& key) const noexcept {
			// TODO: is this hash function any good? Probably not.
			std::hash<uint32_t> h;
			return h(key.level) ^ (h(key.x) << 1) ^ (h(key.y) << 2);
		}
	};


	/**
	 * @brief A hash function for {@link CesiumGeometry::OctreeTileID} objects.
	 */
	template <> struct hash<CesiumGeometry::UpsampledQuadtreeNode> {

		/**
		 * @brief A specialization of the `std::hash` template for
		 * {@link CesiumGeometry::OctreeTileID} objects.
		 */
		size_t operator()(const CesiumGeometry::UpsampledQuadtreeNode& key) const noexcept {
			// TODO: is this hash function any good? Probably not.
			std::hash<uint32_t> h;
			return h(key.tileID.level) ^ (h(key.tileID.x) << 1) ^ (h(key.tileID.y) << 2);
		}
	};
}

#endif // !CESIUM_VARIANT_HASH_H
