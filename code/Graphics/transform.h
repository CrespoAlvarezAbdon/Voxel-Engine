/**
* @file transform.h
* @version 1.0
* @date 26/03/2024
* @author Abdon Crespo Alvarez
* @title Transform.
* @brief Contains the transform struct.
*/
#ifndef _VOXELENG_TRANSFORM_
#define _VOXELENG_TRANSFORM_

#include <cmath>
#include "../definitions.h"
#include "../vec.h"

namespace VoxelEng {

	/**
	* @brief A transform represents the position, rotation and scale of something in the world.
	*/
	struct transform {

		/*
		Methods.
		*/

		// Constructors.

		/**
		* @brief Default constructor.
		*/
		transform()
		: position(vec3Zero), chunkPosition(vec3Zero), rotation(vec3Zero), scale{1.0f, 1.0f, 1.0f},
		  Xaxis(vec3FixedNorth), Yaxis(vec3FixedUp), Zaxis(vec3FixedEast),
		  viewDirection(vec3FixedNorth),
		  gravityDirection(vec3FixedDown)
		{}


		// Observers.

		/**
		* @brief Get sin(transform's current rotation angle) in X-axis
		*/
		float sinRotX() const;

		/**
		* @brief Get cos(transform's current rotation angle) in X-axis.
		*/
		float cosRotX() const;

		/**
		* @brief Get sin(transform's current rotation angle) in Y-axis.
		*/
		float sinRotY() const;

		/**
		* @brief Get cos(transform's current rotation angle) in Y-axis.
		*/
		float cosRotY() const;

		/**
		* @brief Get sin(transform's current rotation angle) in Z-axis.
		*/
		float sinRotZ() const;

		/**
		* @brief Get cos(transform's current rotation angle) in Z-axis.
		*/
		float cosRotZ() const;

		/**
		* @brief Get the transform's rotation axes and angles while comparing the fixed Up vector with the current Y axis.
		*/
		void getRotationFromYaxis(vec3& axis, float& angle) const;


		/*
		Attributes.
		*/

		vec3 position,
			 chunkPosition,
			 rotation,
			 scale,
			 Xaxis,
			 Yaxis,
			 Zaxis,
			 viewDirection,
			 gravityDirection;

	};

	inline float transform::sinRotX() const {

		return std::sin(rotation.x * piDiv);

	}

	inline float transform::cosRotX() const {

		return std::cos(rotation.x * piDiv);

	}

	inline float transform::sinRotY() const {

		return std::sin(rotation.y * piDiv);

	}

	inline float transform::cosRotY() const {

		return std::cos(rotation.y * piDiv);

	}

	inline float transform::sinRotZ() const {

		return std::sin(rotation.z * piDiv);

	}

	inline float transform::cosRotZ() const {
		
		return std::cos(rotation.z * piDiv);

	}

}

#endif