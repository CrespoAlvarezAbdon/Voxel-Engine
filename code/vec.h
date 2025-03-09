/**
* @file vec.h
* @version 1.0
* @date 02/12/2023
* @author Abdon Crespo Alvarez
* @title Vec.
* @brief Contains the definition of the mathematical concept of vector.
*/
#ifndef _VOXELENG_VEC_
#define _VOXELENG_VEC_

#include <unordered_map>
#include <string>
#include <definitions.h>

#if GRAPHICS_API == OPENGL

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <hash.hpp>

#endif


namespace VoxelEng {

	// Type definitions.

	#if GRAPHICS_API == OPENGL

		typedef glm::vec2 vec2;
		typedef glm::vec3 vec3;
		typedef glm::vec4 vec4;

	#else



	#endif

	struct basic_vec4 
	{
		char x;
		char y;
		char z;
		char w;

		basic_vec4();

		basic_vec4(char x, char y, char z, char w);

		/**
		* @brief Perform component-based addition of two vectors.
		* @param v The right operand vector.
		* @return A vector with the result of the addition.
		*/
		basic_vec4 operator+(const basic_vec4 v2) const;

		/**
		* @brief Component-based add the right operand vector to the left operand vector.
		* @param v The right operand vector.
		* @return The left operand vector.
		*/
		basic_vec4& operator+=(const basic_vec4 v2);

		/**
		* @brief Multiply the vector's component by the given number.
		* The multiplication will be done with the given number's type and then cast back to the vector components' type.
		* @param scale The given number.
		* @return A vector with the result of this operation.
		*/
		basic_vec4 operator*(float scalar) const;

		/**
		* @brief Divide the vector's component by the given number.
		* The multiplication will be done with the given number's type and then cast back to the vector components' type.
		* @param scale The given number.
		* @return A vector with the result of this operation.
		*/
		basic_vec4 operator/(char scalar) const;

	};

	inline basic_vec4::basic_vec4()
	: x(0), y(0), z(0), w(0)
	{}

	inline basic_vec4::basic_vec4(char x, char y, char z, char w) 
	: x(x), y(y), z(z), w(w)
	{}

	inline basic_vec4 basic_vec4::operator+(basic_vec4 v) const {

		return basic_vec4{ x + v.x, y + v.y, z + v.z, w + v.w };

	}

	inline basic_vec4& basic_vec4::operator+=(basic_vec4 v) {

		x += v.x;
		y += v.y;
		z += v.z;
		return *this;

	}

	inline basic_vec4 basic_vec4::operator*(float scalar) const {
	
		return basic_vec4{ static_cast<char>(x * scalar), static_cast<char>(y * scalar), static_cast<char>(z * scalar), static_cast<char>(w * scalar) };
	
	}

	inline basic_vec4 basic_vec4::operator/(char scalar) const {

		return basic_vec4{ x / scalar, y / scalar, z / scalar, w / scalar };

	}


	// Constants.

	/**
	* @brief vec3 constant of the zero vector.
	*/
	const vec3 vec3Zero(0, 0, 0);
	/**
	* @brief vec3 constant poiting to the fixed up direction.
	*/
	const vec3 vec3FixedUp(0, 1, 0);
	/**
	* @brief vec3 constant poiting to the fixed down direction.
	*/
	const vec3 vec3FixedDown(0, -1, 0);
	/**
	* @brief vec3 constant poiting to the fixed north direction.
	*/
	const vec3 vec3FixedNorth(1, 0, 0);
	/**
	* @brief vec3 constant poiting to the fixed south direction.
	*/
	const vec3 vec3FixedSouth(-1, 0, 0);
	/**
	* @brief vec3 constant poiting to the fixed east direction.
	*/
	const vec3 vec3FixedEast(0, 0, 1);
	/**
	* @brief vec3 constant poiting to the fixed west direction.
	*/
	const vec3 vec3FixedWest(0, 0, -1);
	/**
	* @brief vec4 filled only with zeroes.
	*/
	const vec4 vec4Zeroes(0, 0, 0, 0);
	/**
	* @brief vec4 filled only with ones.
	*/
	const vec4 vec4Ones(1, 1, 1, 1);


	// Operators.

	/**
	* @brief Returns true if at least one of the components of v1
	* is less than the corresponding component of v2 and the rest
	* of v1 are less than or equal to the correspondings of v2.
	*/
	inline bool operator< (const vec3& v1, const vec3& v2) {

		return (v1.x < v2.x && v1.y <= v2.y && v1.z <= v2.z) ||
			   (v1.x <= v2.x&& v1.y < v2.y && v1.z <= v2.z) ||
			   (v1.x <= v2.x&& v1.y <= v2.y && v1.z < v2.z);

	}

}

namespace std {

	inline std::string to_string(const VoxelEng::vec3& v) {
	
		return std::to_string(v.x) + ',' + std::to_string(v.y) + ',' + std::to_string(v.z);
	
	}

}

#endif