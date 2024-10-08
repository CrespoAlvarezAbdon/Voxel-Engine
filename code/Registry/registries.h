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
#include <Registry/RegistryInsOrdered/registryInsOrdered.h>
#include <Utilities/Var/var.h>

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
		static void reset();


		// Observers.

		/**
		* @brief Get whether the registries collection has been initialised or not.
		* @returns Whether the registries collection has been initialised or not.
		*/
		static bool initialised();

		/**
		* @brief Get the specified registry.
		* @param name The name of the specified registry.
		* @returns The specified registry. Throws exception if said registry is not registered.
		*/
		static const var* getC(const std::string& name);

		/**
		* @brief Get the specified insertion-ordered registry.
		* @param name The name of the specified registry.
		* @returns The specified registry. Throws exception if said registry is not registered.
		*/
		static const var* getCInsOrdered(const std::string& name);


		// Modifiers.
		
		/**
		* @brief Get the specified registry.
		* @param name The name of the specified registry.
		* @returns The specified registry. Throws exception if said registry is not registered.
		*/
		static var* get(const std::string& name);

		/**
		* @brief Get the specified insertion-ordered registry.
		* @param name The name of the specified registry.
		* @returns The specified registry. Throws exception if said registry is not registered.
		*/
		static var* getInsOrdered(const std::string& name);

	private:

		static bool initialised_;
		static registry<std::string, var>* registries_;
		static registry<std::string, var>* registriesInsOrdered_;

	};

	inline bool registries::initialised() {
	
		return initialised_;
	
	}

	inline const var* registries::getC(const std::string& name) {

		return get(name);

	}

	inline const var* registries::getCInsOrdered(const std::string& name) {

		return getInsOrdered(name);

	}

}

#endif