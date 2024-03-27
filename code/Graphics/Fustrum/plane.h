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
	* @brief A mathematical plane can be defined with a normal vector and a distance to the origin. Said distance
	* can be defined with a point.
	*/
	class plane {

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
		* @brief Returns the plane's last generated vertices.
		* NOTE. After constructing the plane, it has no generated vertices, so at least one call to plane::generateVertices() should
		* be made before calling this method.
		*/
		const model& vertices() const;


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

		/**
		* @brief Generate (or regenerate if already created) the vertices to draw the plane at the specified position.
		* Since planes are mathematically defined as infinite, 'planeSize' is provided to delimit it.
		*/
		void generateVertices(float planeSize);


		// Deinitializers.

		static void reset();


	private:

		static bool initialized_;
		static const model* planeVertices_;
		static const modelTriangles* planeTriangles_;

		vec3 normal_;
		float distance_;
		model model_;
		transform transform_;

	};

	inline const model& plane::vertices() const {
	
		return model_;
	
	}

}

#endif