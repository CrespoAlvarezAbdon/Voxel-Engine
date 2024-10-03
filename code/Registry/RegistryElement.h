/**
* @file registryElement.h
* @version 1.0
* @date 13/08/2024
* @author Abdon Crespo Alvarez
* @title registryElement.
* @brief Contains the definition of the registryElement class.
*/
#ifndef _VOXELENG_REGISTRYELEMENT_
#define _VOXELENG_REGISTRYELEMENT_

#include <string>

namespace VoxelEng {

	/**
	* @brief A registry element is a type of object that can be registered inside a Registry object.
	* This class provides the basic capabilities that all objects that are to be registered inside Registry objects
	* must have. Each class that ultimately derives from 'registryElement' must define its own static std::string typeName_ private member
	* and its own init() static method to initialise atleast said member.
	*/
	class registryElement {

	public:

		// Initialisation.

		/**
		* @brief Initialise the registryElement system.
		*/
		static void init(const std::string& typeName);


		// Observers.

		/**
		* @brief Get the registry element's typename.
		*/
		static const std::string& typeName();

	private:

		static bool initialised_;
		static std::string typeName_;

	};

	inline const std::string& registryElement::typeName() {

		return typeName_;

	}

}

#endif