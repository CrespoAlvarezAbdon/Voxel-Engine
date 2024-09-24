/**
* @file materials.h
* @version 1.0
* @date 13/08/2024
* @author Abdon Crespo Alvarez
* @title materials.
* @brief Contains the definition of the material class.
*/
#ifndef _VOXELENG_MATERIALS_
#define _VOXELENG_MATERIALS_

#include <string>
#include <Registry/registryElement.h>

namespace VoxelEng {

	//////////////
	//Constants.//
	//////////////

	const unsigned int MAX_MATERIALS = 256; // TODO. MAKE THIS NUMBER DYNAMIC IN TERMS OF HOW MANY MATERIALS ARE REGISTERED AT ENGINE'S GRAPHICAL MODE STARTUP.


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
		* @brief Default constructor.
		*/
		material();

		/**
		* @brief Class constructor.
		* @param ambientR Percentage of red color reflected when hit by ambient lighting.
		* @param ambientG Percentage of green color reflected when hit by ambient lighting.
		* @param ambientB Percentage of blue color reflected when hit by ambient lighting.
		* @param diffuseR Percentage of red color reflected when hit by diffuse lighting.
		* @param diffuseG Percentage of green color reflected when hit by diffuse lighting.
		* @param diffuseB Percentage of blue color reflected when hit by diffuse lighting.
		* @param specularR Percentage of red color reflected when hit by specular lighting.
		* @param specularG Percentage of green color reflected when hit by specular lighting.
		* @param specularB Percentage of blue color reflected when hit by specular lighting.
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

	private:

		static bool initialised_;
		static std::string typeName_;

		float ambient_[4];
		float diffuse_[4];
		float specular_[4];
		float shininess_[4]; // Only the first value is valid. The rest are for padding.

	};

	inline material::material()
	: ambient_{ 1.0f, 1.0f, 1.0f, 1.0f }, diffuse_{ 1.0f, 1.0f, 1.0f, 1.0f }, specular_{ 1.0f, 1.0f, 1.0f, 1.0f }, shininess_{ 32.0f, 1.0f, 1.0f, 1.0f }
	{}

	inline material::material(float ambientR, float ambientG, float ambientB,
		float diffuseR, float diffuseG, float diffuseB,
		float specularR, float specularG, float specularB,
		float shininess)
	: ambient_{ ambientR, ambientG, ambientB, 1.0f }, diffuse_{ diffuseR, diffuseG, diffuseB, 1.0f }, specular_{ specularR, specularG, specularB, 1.0f }, shininess_{ shininess, 0.0f, 0.0f, 0.0f }
	{}

	inline const std::string& material::typeName() {

		return typeName_;

	}

}

#endif