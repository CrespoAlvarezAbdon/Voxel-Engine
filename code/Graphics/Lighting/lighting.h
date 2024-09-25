#ifndef _VOXELENG_LIGHTING_
#define _VOXELENG_LIGHTING_

#include <vec.h>

namespace VoxelEng {

	class lighting {

	public:

		// Observers.

		/**
		* @brief Get the global ambient lighting.
		* @returns The global ambient lighting.
		*/
		//const vec3& ambientLighting() const;

		
		// Modifiers.

		/**
		* @brief Set the global ambient lighting's red color component.
		* @param r The ambient lighting's new red color component.
		*/
		//void ambientLighting(float r);

		/**
		* @brief Set the global ambient lighting's green component.
		* @param g The ambient lighting's new green component.
		*/
		//void ambientLighting(float g);

		/**
		* @brief Set the global ambient lighting's blue color component.
		* @param b The ambient lighting's new blue color component.
		*/
		//void ambientLighting(float b);

		/**
		* @brief Set the global ambient lighting's red, blue and green color components.
		* @param r The ambient lighting's new red color component.
		* @param g The ambient lighting's new green component.
		* @param b The ambient lighting's new blue color component.
		*/
		//void ambientLighting(float r, float g, float b);

		/**
		* @brief Send the current value of the global ambient lighting to the GPU.
		*/
		//void updateAmbientLighting();

	private:

		static vec3 ambientLighting_; // For now, there is only one global ambient lighting.

	};

}

#endif