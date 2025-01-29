#ifndef VECTOR_UTILS_H
#define VECTOR_UTILS_H

#if defined(CESIUM_GD_EXT)
#include <godot_cpp/core/math.hpp>
#include <godot_cpp/variant/vector3.hpp>
#include <godot_cpp/variant/quaternion.hpp>
using namespace godot;
#elif defined(CESIUM_GD_MODULE)
#include "core/math/vector3.h"
#include <core/math/math_funcs.h>
#include "core/math/quaternion.h"
#endif

#include <glm/detail/type_vec3.hpp>
#include <glm/detail/type_quat.hpp>
#include <glm/detail/type_mat3x3.hpp>
#include <glm/fwd.hpp>
#include <array>
#include <vector>

/**
 * @brief Provides utility functions to calculate operations for glm vectors from GD native vectors and vice versa
 */
class CesiumMathUtils {
public:
	static inline Vector3 from_glm_vec3(const glm::vec3& vec) {
		return Vector3(vec.x, vec.y, vec.z);
	}

	static inline Vector2 from_glm_vec2(const glm::dvec2& vec) {
		return Vector2(vec.x, vec.y);
	}

	static inline Vector2 from_glm_vec2(const glm::vec2& vec) {
		return Vector2(vec.x, vec.y);
	}

	static inline Vector3 from_glm_vec3(const glm::dvec3& vec) {
		return Vector3(vec.x, vec.y, vec.z);
	}

	static inline glm::vec3 to_glm_vec3(const Vector3& vec) {
		return glm::vec3(vec.x, vec.y, vec.z);
	}

	static inline glm::dvec3 to_glm_dvec3(const Vector3& vec) {
		return glm::dvec3(vec.x, vec.y, vec.z);
	}

	static inline glm::vec2 to_glm_vec2(const Vector2& vec) {
		return glm::vec2(vec.x, vec.y);
	}

	static inline glm::vec3 normalized(const glm::vec3& vec) {
		real_t magnitude = Math::sqrt(vec.x * vec.x + vec.y * vec.y + vec.z * vec.z);
		return glm::vec3(vec.x / magnitude, vec.y / magnitude, vec.z / magnitude);
	}

	static inline constexpr glm::dvec3 to_enu_vector(const Vector3& vec) {
		return glm::dvec3(-vec.z, vec.x, vec.y);
	}

	static inline constexpr glm::dvec3 cesium_up() {
		return glm::dvec3(0.0, 0.0, 1.0);
	}

	static inline constexpr glm::dvec3 cesium_down() {
		return glm::dvec3(0.0, 0.0, -1.0);
	}

	static inline constexpr glm::vec3 up() {
		return glm::vec3(0.0f, 1.0f, 0.0f);
	}

	static inline constexpr glm::vec3 down() {
		return glm::vec3(0.0f, -1.0f, 0.0f);
	}

	static inline glm::dvec3 mult_glm(const glm::dvec3& vec, real_t scalar) {
		return glm::dvec3(vec.x * scalar, vec.y * scalar, vec.z * scalar);
	}

	static inline glm::dvec3 engine_to_ecef(const Vector3& vec) {
		return glm::dvec3();
	}

	static inline glm::dmat4 array_to_dmat4(const std::vector<double>& vec) {
		if (vec.size() != 16) {
			ERR_PRINT("Array must have exactly 16 elements to convert it to a matrix, # of elements found : " + itos(vec.size()));
		}

		glm::mat4 mat;
		for (int32_t i = 0; i < 4; ++i) {
			for (int32_t j = 0; j < 4; ++j) {
				mat[i][j] = vec[i * 4 + j];
			}
		}

		return mat;
	}

	static inline std::array<real_t, 9> extract_rotation_matrix(const std::vector<double>& matrix) {
		// Extract the upper-left 3x3 submatrix
		std::array<real_t, 9> rotationMatrix = {
			static_cast<real_t>(matrix[0]), static_cast<real_t>(matrix[4]), static_cast<real_t>(matrix[8]),
			static_cast<real_t>(matrix[1]), static_cast<real_t>(matrix[5]), static_cast<real_t>(matrix[9]),
			static_cast<real_t>(matrix[2]), static_cast<real_t>(matrix[6]), static_cast<real_t>(matrix[10])
		};
		return rotationMatrix;
	}

	static inline Quaternion from_glm_quat(const glm::dquat& q) {
		return Quaternion(q.x, q.y, q.z, q.w);
	}

};

#endif // !VECTOR_UTILS_H
