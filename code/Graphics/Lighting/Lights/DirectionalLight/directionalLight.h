#ifndef _VOXELENG_DIRECTIONAL_LIGHTING_
#define _VOXELENG_DIRECTIONAL_LIGHTING_

#include <vec.h>
#include <Registry/registryElement.h>
#include <Graphics/Lighting/lights/light.h>

namespace VoxelEng {

	class directionalLight : public light {

	public:

		// Constructors.

		/**
		* @brief Default class constructor.
		*/
		directionalLight();

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
		directionalLight(float ambientR, float ambientG, float ambientB,
			float diffuseR, float diffuseG, float diffuseB,
			float specularR, float specularG, float specularB);


		// Observers.

		/**
		* @brief Get the number of arguments needed to construct an object of this class.
		* @returns The number of arguments needed to construct an object of this class.
		*/
		static unsigned int nArgs();

	protected:

		static const unsigned int nArgs_;
		
	};

	inline directionalLight::directionalLight()
	: light()
	{}

	inline directionalLight::directionalLight(float ambientR, float ambientG, float ambientB, 
		float diffuseR, float diffuseG, float diffuseB,
		float specularR, float specularG, float specularB)
	: light(ambientR, ambientG, ambientB, diffuseR, diffuseG, diffuseB, specularR, specularG, specularB)
	{}

	inline unsigned int directionalLight::nArgs() {

		return light::nArgs() + nArgs_;

	}

}

#endif