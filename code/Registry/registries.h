/**
* @file registries.h
* @version 1.0
* @date 13/08/2024
* @author Abdon Crespo Alvarez
* @title Registries.
* @brief Contains the registries used in the engine.
*/
#ifndef _VOXELENG_REGISTRIES_
#define _VOXELENG_REGISTRIES_

#include <string>
#include "RegistryInsOrdered/registryInsOrdered.h"
#include "../Graphics/Materials/materials.h"

namespace VoxelEng {

	/**
	* @brief Static class that holds all the engine's registries.
	*/
	class registries {

	public:
		
		// Initialisation.

		/**
		* @brief Initialise the registries collection.
		*/
		static void init();

		/**
		* @brief Deinitialise the registries collection.
		*/
		static void deinit();


		// Modifiers.

		/**
		* @brief Get the vertex graphical materials registry.
		*/
		static registryInsOrdered<std::string, material>& materials();

	private:

		static bool initialised_;
		static registryInsOrdered<std::string, material>* materials_;

	};

	inline registryInsOrdered<std::string, material>& registries::materials() {
	
		return *materials_;
	
	}

}

#endif