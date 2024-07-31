#ifndef _VOXELENG_MATERIALS_
#define _VOXELENG_MATERIALS_

#include <unordered_map>
#include <string>

namespace VoxelEng {

	const unsigned int MAX_MATERIALS = 128; // TODO. MAKE THIS NUMBER DYNAMIC IN TERMS OF HOW MANY MATERIALS ARE REGISTERED AT ENGINE'S GRAPHICAL MODE STARTUP.

	/**
	* @brief Represents the properties of a 3D vertex such as the reflected color
	* when the fragments affected by said vertex are hit by ambient, diffuse or specular lighting and
	* the specular shininess to use.
	*/
	class material {

	public:

		/*
		Methods.
		*/

		// Observers.

		/**
		* @brief Get the previously registered material specified by the given name as a constant reference.
		* @param name The material's unique name.
		* @return The specified material.
		*/
		static const material& getMaterial(const std::string& name);


		// Modifiers.

		/**
		* @brief Register new material with the given unique name.
		* @param name The material's unique name.
		* @param ambientR Percentage of red color reflected when hit by ambient lighting.
		* @param ambientG Percentage of green color reflected when hit by ambient lighting.
		* @param ambientB Percentage of blue color reflected when hit by ambient lighting.
		* @param diffuseR Percentage of red color reflected when hit by diffuse lighting.
		* @param diffuseG Percentage of green color reflected when hit by diffuse lighting.
		* @param diffuseB Percentage of blue color reflected when hit by diffuse lighting.
		* @param shininess Specular shininess.
		*/
		static void registerMaterial(const std::string& name, 
			float ambientR = 1.0f, float ambientG = 1.0f, float ambientB = 1.0f,
			float diffuseR = 1.0f, float diffuseG = 1.0f, float diffuseB = 1.0f,
			float specularR = 1.0f, float specularG = 1.0f, float specularB = 1.0f, float shininess = 32.0f);

		/**
		* @brief Get the previously registered material specified by the given name as a non-constant reference.
		* @param name The material's unique name.
		* @return The specified material.
		*/
		static material& setMaterial(const std::string& name);

		/**
		* @brief Unregisters a previously registered material.
		* @param name The material's unique name.
		*/
		static void unregisterMaterial(const std::string& name);

		// NEXT.
		// 1º. DEFINE CONSTRUCTORS. DONE.
		// 2º. MAKE A STATIC UNORDERED_MAP<STRING, MATERIAL> WHERE STRING IS NAMESPACED ID. DONE.
		// 3º. ADD CRUD OPERATIONS TO SAID MAP.


		/*
		Attributes.
		*/

		float ambient[3];
		float diffuse[3];
		float specular[3];
		float shininess;

	private:

		/*
		Methods.
		*/

		// Constructors.

		material();

		material(float ambientR, float ambientG, float ambientB,
			float diffuseR, float diffuseG, float diffuseB,
			float specularR, float specularG, float specularB, float shininess);

		/*
		Attributes.
		*/

		static std::unordered_map<std::string, material> materials_;

	};

}

#endif