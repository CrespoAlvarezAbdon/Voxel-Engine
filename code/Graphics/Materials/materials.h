#ifndef _VOXELENG_MATERIALS_
#define _VOXELENG_MATERIALS_

#include <string>
#include "../../Registry/registryElement.h"

namespace VoxelEng {

	//////////////
	//Constants.//
	//////////////

	const unsigned int MAX_MATERIALS = 128; // TODO. MAKE THIS NUMBER DYNAMIC IN TERMS OF HOW MANY MATERIALS ARE REGISTERED AT ENGINE'S GRAPHICAL MODE STARTUP.


	////////////
	//Classes.//
	////////////

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

		/**
		* @brief Initialise the vertex graphic materials system.
		*/
		static void init(const std::string& typeName);


		// Constructors.

		/**
		* @brief Class constructor.
		* @param ambientR Percentage of red color reflected when hit by ambient lighting.
		* @param ambientG Percentage of green color reflected when hit by ambient lighting.
		* @param ambientB Percentage of blue color reflected when hit by ambient lighting.
		* @param diffuseR Percentage of red color reflected when hit by diffuse lighting.
		* @param diffuseG Percentage of green color reflected when hit by diffuse lighting.
		* @param diffuseB Percentage of blue color reflected when hit by diffuse lighting.
		* @param shininess Specular shininess.
		*/
		material(float ambientR, float ambientG, float ambientB,
			float diffuseR, float diffuseG, float diffuseB,
			float specularR, float specularG, float specularB, float shininess);


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

	inline material::material(float ambientR, float ambientG, float ambientB,
		float diffuseR, float diffuseG, float diffuseB,
		float specularR, float specularG, float specularB,
		float shininess)
		: ambient{ ambientR, ambientG, ambientB }, diffuse{ diffuseR, diffuseG, diffuseB }, specular{ specularR, specularG, specularB }, shininess(shininess)
	{}

	inline const std::string& material::typeName() {

		return typeName_;

	}

}

#endif