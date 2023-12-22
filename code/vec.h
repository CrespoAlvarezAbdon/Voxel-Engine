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

#include <string>
#include "definitions.h"

#if GRAPHICS_API == OPENGL

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm.hpp>

#endif


namespace VoxelEng {

	// Type definitions.

	#if GRAPHICS_API == OPENGL

		typedef glm::vec3 vec3;
		typedef glm::vec2 vec2;

	#else



	#endif


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