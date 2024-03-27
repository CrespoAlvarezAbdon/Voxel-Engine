/**
* @file plane.h
* @version 1.0
* @date 21/03/2024
* @author Abdon Crespo Alvarez
* @title Fustrum.
* @brief Mathematical definition of a fustrum.
*/
#ifndef _VOXELENG_FUSTRUM_
#define _VOXELENG_FUSTRUM_

#include "plane.h"
#include "../../camera.h"


namespace VoxelEng {

	/**
	* @brief Representation of a fustrum as a portion of a pyramid. Used in cameras to
	* define the portion of the world that they can render.
	*/
	class fustrum {
	public:

		// Constructors.

		/**
		* @brief Constructs a fustrum with the given camera's parameters.
		*/
		fustrum(const camera& cam);

	private:

		plane near_; // zNear
		plane far_; // zFar
		plane right_;
		plane left_;
		plane top_;
		plane bottom_;
		
	};

}

#endif