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
#include <memory>
#include "RegistryInsOrdered/registryInsOrdered.h"
#include <Graphics/Materials/materials.h>
#include <Graphics/Lighting/Lights/DirectionalLight/directionalLight.h>
#include <Graphics/Lighting/Lights/PointLight/pointLight.h>
#include <Graphics/Lighting/Lights/SpotLight/spotLight.h>

namespace VoxelEng {

	// TODO. MAKE THIS A SINGLETON.

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
		* @brief Get the vertex graphical material types registry.
		*/
		static registryInsOrdered<std::string, material>& materials();

		/**
		* @brief Get the graphical light types registry.
		*/
		static registryInsOrdered<std::string, directionalLight>& directionalLights();

		/**
		* @brief Get the graphical light types registry.
		*/
		static registryInsOrdered<std::string, pointLight>& pointLights();

		/**
		* @brief Get the graphical light types registry.
		*/
		static registryInsOrdered<std::string, spotLight>& spotLights();

	private:

		static bool initialised_;
		static registryInsOrdered<std::string, material>* materials_;
		static registryInsOrdered<std::string, directionalLight>* directionalLights_;
		static registryInsOrdered<std::string, pointLight>* pointLights_;
		static registryInsOrdered<std::string, spotLight>* spotLights_;

	};

	inline registryInsOrdered<std::string, material>& registries::materials() {
	
		return *materials_;
	
	}

	inline registryInsOrdered<std::string, directionalLight>& registries::directionalLights() {

		return *directionalLights_;

	}

	inline registryInsOrdered<std::string, pointLight>& registries::pointLights() {

		return *pointLights_;

	}

	inline registryInsOrdered<std::string, spotLight>& registries::spotLights() {

		return *spotLights_;

	}

}

#endif