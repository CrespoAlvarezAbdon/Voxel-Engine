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

#include <stdexcept>
#include <string>
#include <memory>
#include "RegistryInsOrdered/registryInsOrdered.h"

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
		* @brief Get the specified registry.
		* @param name The name of the specified registry.
		* @returns The specified registry. Throws exception if said registry is not registered.
		*/
		template <typename T>
		requires std::derived_from<T, registryElement>
		static registry<std::string, T>& get(const std::string& name);

		/**
		* @brief Get the specified insertion-ordered registry.
		* @param name The name of the specified registry.
		* @returns The specified registry. Throws exception if said registry is not registered.
		*/
		template <typename T>
		requires std::derived_from<T, registryElement>
		static registryInsOrdered<std::string, T>& getInsOrdered(const std::string& name);

	private:

		static bool initialised_;
		static registry<std::string, registry<std::string, registryElement>>* registries_;
		static registry<std::string, registryInsOrdered<std::string, registryElement>>* registriesInsOrdered_;

	};

	template <typename T>
	requires std::derived_from<T, registryElement>
	registry<std::string, T>& registries::get(const std::string& name) {
	
		if (registries_->contains(name))
		{
			return *registries_->get(name);
		}
		else
			throw std::runtime_error("The registry " + name + " is not registered");
	
	}

	template <typename T>
	requires std::derived_from<T, registryElement>
	registryInsOrdered<std::string, T>& registries::getInsOrdered(const std::string& name) {

		if (registriesInsOrdered_->contains(name))
		{
			return *registriesInsOrdered_->get(name);
		}
		else
			throw std::runtime_error("The insertion-ordered registry " + name + " is not registered");

	}

}

#endif