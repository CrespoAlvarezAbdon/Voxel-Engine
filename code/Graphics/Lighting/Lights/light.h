#ifndef _VOXELENG_LIGHT_
#define _VOXELENG_LIGHT_

#include <vec.h>
#include <Registry/registryElement.h>

namespace VoxelEng {

	//////////////
	//Constants.//
	//////////////

	const unsigned int MAX_LIGHTS = 256; // TODO. MAKE THIS NUMBER DYNAMIC IN TERMS OF HOW MANY DIFFERENT TYPES OF LIGHT ARE REGISTERED AT ENGINE'S GRAPHICAL MODE STARTUP.

	class light : public registryElement {

	public:

		// Initialisation.

		/**
		* @brief Initialise the registryElement system.
		*/
		static void init(const std::string& typeName);


		// Constructors.

		/**
		* @brief Default class constructor.
		*/
		light();

		/**
		* @brief Class constructor.
		* @param ambientR Percentage of red color emitted in ambient lighting calculations by this light type.
		* @param ambientG Percentage of green color emitted in ambient lighting calculations by this light type.
		* @param ambientB Percentage of blue color emitted in ambient lighting calculations by this light type.
		* @param diffuseR Percentage of red color emitted in diffuse lighting calculations by this light type.
		* @param diffuseG Percentage of green color emitted in diffuse lighting calculations by this light type.
		* @param diffuseB Percentage of blue color emitted in diffuse lighting calculations by this light type.
		* @param specularR Percentage of red color emitted in specular lighting calculations by this light type.
		* @param specularG Percentage of green color emitted in specular lighting calculations by this light type.
		* @param specularB Percentage of blue color emitted in specular lighting calculations by this light type.
		*/
		light(float ambientR, float ambientG, float ambientB,
			  float diffuseR, float diffuseG, float diffuseB,
			  float specularR, float specularG, float specularB);


		// Observers.

		/**
		* @brief Get the registry element's typename.
		*/
		static const std::string& typeName();

		/**
		* @brief Get the number of arguments needed to construct an object of this class.
		* @returns The number of arguments needed to construct an object of this class.
		*/
		static unsigned int nArgs();

	protected:

		static bool initialised_;
		static std::string typeName_;
		static const unsigned int nArgs_;

		float ambient_[4];
		float diffuse_[4];
		float specular_[4];

	};

	inline light::light()
	: ambient_{1.0f, 1.0f, 1.0f, 1.0f,}, diffuse_{ 1.0f,  1.0f, 1.0f, 1.0f }, specular_{ 1.0f, 1.0f, 1.0f, 1.0f }
	{}

	inline light::light(float ambientR, float ambientG, float ambientB, 
		float diffuseR, float diffuseG, float diffuseB,
		float specularR, float specularG, float specularB) 
	: ambient_{ ambientR, ambientG, ambientB, 1.0f, }, diffuse_{ diffuseR, diffuseG, diffuseB, 1.0f }, specular_{ specularR, specularG, specularB, 1.0f }
	{}

	inline const std::string& light::typeName() {

		return typeName_;

	}

	inline unsigned int light::nArgs() {

		return nArgs_;

	}

}

#endif