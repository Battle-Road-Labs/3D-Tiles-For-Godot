#ifndef VECTOR_UTILS_H
#define VECTOR_UTILS_H

#include "glm/ext/matrix_double4x4.hpp"
#include "glm/ext/matrix_transform.hpp"
#include "glm/ext/vector_double3.hpp"
#include "glm/trigonometric.hpp"
#if defined(CESIUM_GD_EXT)
#include "godot_cpp/core/error_macros.hpp"
#include "godot_cpp/variant/string.hpp"
#include "godot_cpp/variant/transform3d.hpp"
#include "godot_cpp/variant/vector2.hpp"
#include <godot_cpp/core/math.hpp>
#include <godot_cpp/variant/vector3.hpp>
#include <godot_cpp/variant/quaternion.hpp>
using namespace godot;
#else
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
		ERR_FAIL_V_MSG(glm::dvec3(), "engine_to_ecef on MathUtils NOT YET IMPLEMENTED!");
		// return glm::dvec3();
	}

	static inline glm::dmat4 to_glm_mat4(const Transform3D& t) {
		glm::dmat4 result;
	    result[0][0] = t.basis.rows[0].x;
	    result[1][0] = t.basis.rows[1].x;
	    result[2][0] = t.basis.rows[2].x;
	    result[3][0] = t.origin.x;

	    result[0][1] = t.basis.rows[0].y;
	    result[1][1] = t.basis.rows[1].y;
	    result[2][1] = t.basis.rows[2].y;
	    result[3][1] = t.origin.y;

	    result[0][2] = t.basis.rows[0].z;
	    result[1][2] = t.basis.rows[1].z;
	    result[2][2] = t.basis.rows[2].z;
	    result[3][2] = t.origin.z;

	    result[0][3] = 0.0;
	    result[1][3] = 0.0;
	    result[2][3] = 0.0;
	    result[3][3] = 1.0;
		return result;
	}


	static inline Transform3D from_glm_mat4(const glm::dmat4& m) {
	    Transform3D t;
	    
	    t.basis.rows[0].x = m[0][0];
	    t.basis.rows[0].y = m[0][1];
	    t.basis.rows[0].z = m[0][2];

	    t.basis.rows[1].x = m[1][0];
	    t.basis.rows[1].y = m[1][1];
	    t.basis.rows[1].z = m[1][2];

	    t.basis.rows[2].x = m[2][0];
	    t.basis.rows[2].y = m[2][1];
	    t.basis.rows[2].z = m[2][2];

	    t.origin.x = m[3][0];
	    t.origin.y = m[3][1];
	    t.origin.z = m[3][2];

	    return t;
	}
	
	static inline glm::dvec3 ecef_to_engine(const glm::dvec3& vec) {
		constexpr glm::dmat4 identity = glm::dmat4(1.0); 
		constexpr glm::dmat4 translation = glm::translate(identity, glm::dvec3(0.0)); 
		constexpr double angleX = glm::radians(-90.0);
		glm::dmat4 rotation = glm::rotate(identity, angleX, glm::dvec3(1.0, 0.0, 0.0));
		glm::dmat4 scale = glm::scale(identity, glm::dvec3(1.0));
		glm::dmat4 trs = translation * rotation * scale;

		return trs * glm::dvec4(vec, 1.0);
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
	
	static inline glm::dquat to_glm_dquat(const Quaternion& q) {
		return glm::dquat(q.x, q.y, q.z, q.w);
	}

};

#endif // !VECTOR_UTILS_H
