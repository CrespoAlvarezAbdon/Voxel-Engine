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
#include <stdexcept>


#if GRAPHICS_API == OPENGL

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <hash.hpp>

#endif


namespace VoxelEng {

	/////////////////////////
	//Forward declarations.//
	/////////////////////////
	enum class blockViewDir;


	// Type definitions.

	#if GRAPHICS_API == OPENGL

		typedef glm::vec2 vec2;
		typedef glm::vec3 vec3;
		typedef glm::vec4 vec4;

	#else



	#endif

	// Other operators.

	vec3 operator+(const vec3& v, blockViewDir viewDir);


	/**
	* @brief Vector of 3 bytes, 1 byte per component.
	*/
	struct basicVec3
	{
		char x;
		char y;
		char z;

		/**
		* @brief Default class constructor.
		*/
		basicVec3();

		/**
		* @brief Class constructor.
		* @param x First component.
		* @param y Second component.
		* @param z Third component.
		*/
		basicVec3(char x, char y, char z);

		bool operator==(const basicVec3& v) const;

		~basicVec3();
	};

	inline basicVec3::basicVec3()
		: x(0), y(0), z(0)
	{}

	inline basicVec3::basicVec3(char x, char y, char z)
		: x(x), y(y), z(z)
	{}

	inline bool basicVec3::operator==(const basicVec3& v) const {
	
		return x == v.x && y == v.y && z == v.z;
	
	}

	inline basicVec3::~basicVec3() 
	{
		int a = 3 + 2;
	}


	/**
	* @brief Vector of 4 bytes, 1 byte per component.
	*/
	struct basicVec4 
	{
		char x;
		char y;
		char z;
		char w;

		/**
		* @brief Default class constructor.
		*/
		basicVec4();

		/**
		* @brief Class constructor.
		* @param x First component.
		* @param y Second component.
		* @param z Third component.
		* @param w Fourth component.
		*/
		basicVec4(char x, char y, char z, char w);

		/**
		* @brief Perform component-based addition of two vectors.
		* @param v The right operand vector.
		* @return A vector with the result of the addition.
		*/
		basicVec4 operator+(const basicVec4 v) const;

		/**
		* @brief Component-based add the right operand vector to the left operand vector.
		* @param v The right operand vector.
		* @return The left operand vector.
		*/
		basicVec4& operator+=(const basicVec4 v);

		/**
		* @brief Multiply the vector's component by the given number.
		* The multiplication will be done with the given number's type and then cast back to the vector components' type.
		* @param scale The given number.
		* @return A vector with the result of this operation.
		*/
		basicVec4 operator*(float scalar) const;

		/**
		* @brief Divide the vector's component by the given number.
		* The multiplication will be done with the given number's type and then cast back to the vector components' type.
		* @param scale The given number.
		* @return A vector with the result of this operation.
		*/
		basicVec4 operator/(char scalar) const;

		/**
		* @brief Component-based add the right operand vector to the left operand vector.
		* NOTE. If the sum of two components would surpass the data range limit, the result will be clamped to said limit
		* @param v The right operand vector.
		* @return The left operand vector.
		*/
		basicVec4 safeAdd(const basicVec4& v) const;

		/**
		* @brief Component-based add the right operand vector to the left operand vector.
		* NOTE. If the sum of two components would surpass the data range limit, the result will be clamped to said limit
		* @param v The right operand vector.
		* @return The left operand vector.
		*/
		basicVec4& safeAdd(const basicVec4& v);

	};

	inline basicVec4::basicVec4()
	: x(0), y(0), z(0), w(0)
	{}

	inline basicVec4::basicVec4(char x, char y, char z, char w)
	: x(x), y(y), z(z), w(w)
	{}

	inline basicVec4 basicVec4::operator+(basicVec4 v) const {

		return basicVec4{ x + v.x, y + v.y, z + v.z, w + v.w };

	}

	inline basicVec4& basicVec4::operator+=(basicVec4 v) {

		x += v.x;
		y += v.y;
		z += v.z;
		return *this;

	}

	inline basicVec4 basicVec4::operator*(float scalar) const {
	
		return basicVec4{ static_cast<char>(x * scalar), static_cast<char>(y * scalar), static_cast<char>(z * scalar), static_cast<char>(w * scalar) };
	
	}

	inline basicVec4 basicVec4::operator/(char scalar) const {

		return basicVec4{ x / scalar, y / scalar, z / scalar, w / scalar };

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

	/**
	* @brief basicVec3 constant of the zero vector.
	*/
	const basicVec3 basicVec3Zero(0, 0, 0);
	/**
	* @brief basicVec3 constant poiting to the fixed up direction.
	*/
	const basicVec3 basicVec3FixedUp(0, 1, 0);
	/**
	* @brief basicVec3 constant poiting to the fixed down direction.
	*/
	const basicVec3 basicVec3FixedDown(0, -1, 0);
	/**
	* @brief basicVec3 constant poiting to the fixed north direction.
	*/
	const basicVec3 basicVec3FixedNorth(1, 0, 0);
	/**
	* @brief basicVec3 constant poiting to the fixed south direction.
	*/
	const basicVec3 basicVec3FixedSouth(-1, 0, 0);
	/**
	* @brief basicVec3 constant poiting to the fixed east direction.
	*/
	const basicVec3 basicVec3FixedEast(0, 0, 1);
	/**
	* @brief basicVec3 constant poiting to the fixed west direction.
	*/
	const basicVec3 basicVec3FixedWest(0, 0, -1);

	/**
	* @brief basicVec4 constant of the zeroes vector.
	*/
	const basicVec4 basicVec4Zeroes(0, 0, 0, 0);

	/**
	* @brief basicVec4 constant of the negative ones vector.
	*/
	const basicVec4 basicVec4NegOnes(-1, -1, -1, -1);


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

	template <>
	class hash<VoxelEng::basicVec3> {
	public:

		std::size_t operator()(const VoxelEng::basicVec3& v) const
		{
			return std::hash<glm::vec3>()(glm::vec3{v.x, v.y, v.z});
		}

	};

}

#endif