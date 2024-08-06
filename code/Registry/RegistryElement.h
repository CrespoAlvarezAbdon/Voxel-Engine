#ifndef _VOXELENG_REGISTRYELEMENT_
#define _VOXELENG_REGISTRYELEMENT_

#include <string>


namespace VoxelEng {

	/**
	* @brief A registry element is a type of object that can be registered inside a Registry object.
	* This class provides the basic capabilities that all objects that are to be registered inside Registry objects
	* must have. Each class that ultimately derives from 'registryElement' must define its own static std::string typeName_ private member
	* and its own initialize() static method to initialize atleast said member.
	*/
	class registryElement {

	public:

		// Initialization.

		static void initialize(const std::string& typeName);


		// Observers.

		/**
		* @brief Get the registry element's typename.
		*/
		static const std::string& typeName();

	private:

		static const bool initialized_;
		static std::string typeName_;

	};

}

#endif