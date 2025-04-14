#ifndef _VOXELENG_BLOCK_VIEW_DIR_
#define _VOXELENG_BLOCK_VIEW_DIR_

#include <vec.h>

namespace VoxelEng {

	/**
	* @brief Viewing directions expressed in a discrete and simple way. It could be seen as a "blocky" viewing direction
	* if it could be compared to block coordinates, which are only integer coordinates compared to global coordinates.
	*/
	enum class blockViewDir { NONE, PLUSY, NEGY, PLUSX, NEGX, PLUSZ, NEGZ };

	/**
	* @brief Returns 'direction' in a vec3.
	*/
	vec3 directionToVec3(blockViewDir direction);

	/**
	* @brief Returns 'direction' in three floats.
	*/
	void directionToVec3(blockViewDir direction, float& x, float& y, float& z);

	/**
	* @brief Returns 'direction' in as a Block View Direction by rounding to the
	* nearest direction.
	*/
	blockViewDir vec3ToDirection(const vec3& direction);

	/**
	* @brief Returns the rotated direction.
	*/
	blockViewDir rotateDirection(blockViewDir dir, blockViewDir rot);

	/**
	* @brief Returns the inverse direction.
	*/
	blockViewDir inverseDirection(blockViewDir dir);

}

#endif