/**
* @file noise.h
* @version 1.0
* @date 25/04/2023
* @author Abdon Crespo Alvarez
* @title Noise generators.
* @brief Contains the declarations of some noise generators.
*/
#ifndef _VOXELENG_NOISE_
#define _VOXELENG_NOISE_

#if GRAPHICS_API == OPENGL
	
#include "../glm/gtc/noise.hpp"

#endif


namespace VoxelEng {

	////////////
	//Classes.//
	////////////

	/**
	* @brief Contains some noise generators.
	*/
	class noise {

	public:

		// Observers.

		/**
		* @brief Returned number is in range [-1, 1].
		*/
		static float perlin2D(float x, float y);

		/**
		* @brief Returned number is in range [-1, 1].
		*/
		static float perlin3D(float x, float y, float z);

		/**
		* @brief Returned number is in range [-1, 1].
		*/
		static float perlin4D(float x, float y, float z, float t);

		/**
		* @brief Returned number is in range [-1, 1].
		*/
		static float simplex2D(float x, float y);

		/**
		* @brief Returned number is in range [-1, 1].
		*/
		static float simplex3D(float x, float y, float z);

		/**
		* @brief Returned number is in range [-1, 1].
		*/
		static float simplex4D(float x, float y, float z, float t);

	private:

		/*
		Methods.
		*/

		// Constructors.

		noise();

	};

	inline noise::noise() {}

	inline float noise::perlin2D(float x, float y) {

		#if GRAPHICS_API == OPENGL

			return glm::perlin(glm::vec2(x, y));

		#else



		#endif

	}

	inline float noise::perlin3D(float x, float y, float z) {

		#if GRAPHICS_API == OPENGL

			return glm::perlin(glm::vec3(x, y, z));

		#else



		#endif

	}

	inline float noise::perlin4D(float x, float y, float z, float t) {
	
		#if GRAPHICS_API == OPENGL

			return glm::perlin(glm::vec4(x, y, z, t));

		#else

			

		#endif
	
	}

	inline float noise::simplex2D(float x, float y) {

		#if GRAPHICS_API == OPENGL

			return glm::simplex(glm::vec2(x, y));

		#else



		#endif

	}

	inline float noise::simplex3D(float x, float y, float z) {

		#if GRAPHICS_API == OPENGL

			return glm::simplex(glm::vec3(x, y, z));

		#else



		#endif

	}

	inline float noise::simplex4D(float x, float y, float z, float t) {
	
		#if GRAPHICS_API == OPENGL

			return glm::simplex(glm::vec4(x, y, z, t));

		#else

			

		#endif
	
	}

}

#endif