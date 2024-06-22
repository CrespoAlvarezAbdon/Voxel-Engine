/**
* @file frustum.h
* @version 1.0
* @date 21/03/2024
* @author Abdon Crespo Alvarez
* @title Frustum.
* @brief Mathematical definition of a fustrum.
*/
#ifndef _VOXELENG_FRUSTUM_
#define _VOXELENG_FRUSTUM_

#include "../../Math/mathPlane.h"
#include "../../camera.h"


namespace VoxelEng {

	/**
	* @brief Representation of a fustrum as a portion of a pyramid. Used in cameras to
	* define the portion of the world that they can render.
	*/
	class frustum {
	public:

		// Constructors.

		/**
		* @brief Constructs a fustrum with the given camera's parameters.
		*/
		frustum(const camera& cam);


		// Observers.

		/**
		* @brief Spawn plane entities that represent a portion
		* of the current fustrum's planes for debugging purposes.
		*/
		void spawnDebugPlanes() const;

		/**
		* @brief Returns true if the given point is inside the frustum
		* or false otherwise.
		*/
		bool isInside(const vec3& point) const;

	private:

		Math::plane near_;
		Math::plane far_;
		Math::plane right_;
		Math::plane left_;
		Math::plane top_;
		Math::plane bottom_;
		
	};

}

#endif