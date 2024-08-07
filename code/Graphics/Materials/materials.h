#ifndef _VOXELENG_MATERIALS_
#define _VOXELENG_MATERIALS_

#include <unordered_map>
#include <string>
#include "../../Registry/registryElement.h"

namespace VoxelEng {

	const unsigned int MAX_MATERIALS = 128; // TODO. MAKE THIS NUMBER DYNAMIC IN TERMS OF HOW MANY MATERIALS ARE REGISTERED AT ENGINE'S GRAPHICAL MODE STARTUP.

	/**
	* @brief Represents the properties of a 3D vertex such as the reflected color
	* when the fragments affected by said vertex are hit by ambient, diffuse or specular lighting and
	* the specular shininess to use.
	*/
	class material : public registryElement {

	public:

		/*
		Methods.
		*/

		// Initialisation.

		static void init(const std::string& typeName);


		// Constructors.

		/**
		* @brief Default class constructor.
		*/
		material();


		// Observers.

		/**
		* @brief Get the registry element's typename.
		*/
		static const std::string& typeName();


		/*
		Attributes.
		*/

		float ambient[3];
		float diffuse[3];
		float specular[3];
		float shininess;

	private:

		static bool initialised_;
		static std::string typeName_;

	};

	inline material::material()
		: ambient{ 1.0f, 1.0f, 1.0f }, diffuse{ 1.0f, 1.0f, 1.0f }, specular{ 1.0f, 1.0f, 1.0f }, shininess(32)
	{}

	inline const std::string& material::typeName() {

		return typeName_;

	}

}

#endif