/**
* @file mathPlane.h
* @version 1.0
* @date 21/03/2024
* @author Abdon Crespo Alvarez
* @title Plane.
* @brief Definition of a mathematical plane.
*/
#ifndef _VOXELENG_MATH_PLANE_
#define _VOXELENG_MATH_PLANE_

#include "../vec.h"

#if GRAPHICS_API == OPENGL

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm.hpp>

#endif

// TODO. PONER INLINES.

namespace VoxelEng {

	namespace Math {
	
		/**
		* @brief Representation of the mathematical concept of plane.
		*/
		class plane {

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

			/**
			* @brief Returns true if the given point is in front of the plane or false otherwise.
			*/
			bool isPointForward(const vec3& point) const;

			/**
			* @brief Returns true if the given point is behind the plane or false otherwise.
			*/
			bool isPointBehind(const vec3& point) const;


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

			/*
			Attributes.
			*/

			vec3 point_;
			vec3 normal_;
			float dotPointNormal_;

		
			/*
			Methods.
			*/
			void updateDotPointNormal();

		};

		inline plane::plane()
			: point_(vec3Zero), normal_(vec3Zero), dotPointNormal_(0.0f)
		{}

		inline plane::plane(const vec3& point, const vec3& normal)
			: point_(point), normal_(glm::normalize(normal)), dotPointNormal_(0.0f)
		{
		
			updateDotPointNormal();
		
		}

		inline const vec3& plane::point() const {

			return point_;

		}

		inline const vec3& plane::normal() const {

			return normal_;

		}

		inline bool plane::isPointForward(const vec3& point) const {
		
			return glm::dot(point, normal_) - dotPointNormal_ > 0;
		
		}

		inline bool plane::isPointBehind(const vec3& point) const {
		
			return glm::dot(point, normal_) - dotPointNormal_ < 0;
		
		}

		inline void plane::updateDotPointNormal() {
		
			dotPointNormal_ = glm::dot(point_, normal_);
		
		}
	
	}

}

#endif