/**
* @file plane.h
* @version 1.0
* @date 21/03/2024
* @author Abdon Crespo Alvarez
* @title Plane.
* @brief Definition of a mathematical plane.
*/
#ifndef _VOXELENG_PLANE_
#define _VOXELENG_PLANE_

#include "../transform.h"
#include "../../entity.h"
#include "../../vec.h"
#include "../../model.h"

#if GRAPHICS_API == OPENGL

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm.hpp>

#endif

// TODO. PONER INLINES.

namespace VoxelEng {

	/**
	* @brief An entity that represents a 2D plane.
	*/
	class plane : public entity {

	public:

		// Initializers.

		static void init();


		// Constructors.

		/**
		* @brief Class constructor. Creates the XZ plane.
		*/
		plane();

		/**
		* @brief Class constructor that creates a plane based on the given normal vector
		* and the distance from the origin to the plane that can be calculated with the given point that
		* is supposed to specify the position of the plane from the origin.
		*/
		plane(const vec3& point, const vec3& normal);


		// Observers.

		/**
		* @brief Get the plane's normal.
		*/
		const vec3& normal() const;


		// Modifiers.

		/**
		* @brief Change the value of the plane's normal.
		*/
		void normal(const vec3& newValue);

		/**
		* @brief Change the value of the plane's distance to the origin by
		* calculating the dot product between the given point and the plane's current normal.
		*/
		void distance(const vec3& pointWithNewDistance);


		// Deinitializers.

		static void reset();

	private:

		static bool initialized_;

		float distance_;

	};

}

#endif