/**
* @file plane.h
* @version 1.0
* @date 21/03/2024
* @author Abdon Crespo Alvarez
* @title Plane.
* @brief Definition of the plane entity.
*/
#ifndef _VOXELENG_PLANE_
#define _VOXELENG_PLANE_

#include "../entity.h"
#include "../vec.h"
#include "../model.h"
#include "../Graphics/transform.h"
#include "../Math/mathPlane.h"

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

		// Constructors.

		/**
		* @brief Class constructor. Creates the XZ plane.
		*/
		plane();

		/**
		* @brief Class constructor that creates a plane based on the given normal vector and position.
		*/
		plane(const vec3& point, const vec3& normal);


		// Observers.

		/**
		* @brief Get the plane's position.
		*/
		const vec3& point() const;

		/**
		* @brief Get the plane's normal.
		*/
		const vec3& normal() const;


		// Modifiers.

		/**
		* @brief Change the value of the plane's position.
		*/
		void point(const vec3& newPoint);

		/**
		* @brief Change the value of the plane's normal.
		*/
		void normal(const vec3& newValue);

	private:

		Math::plane mathPlane_;

	};

	inline const vec3& plane::point() const {

		return mathPlane_.point();

	}

	inline const vec3& plane::normal() const {

		return mathPlane_.normal();

	}

}

#endif