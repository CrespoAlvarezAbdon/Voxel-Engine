#ifndef _VOXELENG_POINT_LIGHT_
#define _VOXELENG_POINT_LIGHT_

#include <definitions.h>
#include <vec.h>
#include <Registry/registryElement.h>
#include <Graphics/Lighting/lights/light.h>

namespace VoxelEng {

	class pointLight : public light {

	public:

		// Constructors.

		/**
		* @brief Default class constructor.
		*/
		pointLight();

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
		* @param maxDistance Maximum distance this light can cover.
		*/
		pointLight(float ambientR, float ambientG, float ambientB, 
			float diffuseR, float diffuseG, float diffuseB,
			float specularR, float specularG, float specularB,
			float maxDistance);


		// Observers.

		/**
		* @brief Get the number of arguments needed to construct an object of this class.
		* @returns The number of arguments needed to construct an object of this class.
		*/
		static unsigned int nArgs();

	protected:

		static const unsigned int nArgs_;

		float maxDistance_;
		float padding_[3];
		
	};

	inline pointLight::pointLight()
	: light(), maxDistance_(defaultLightMaxDistance), padding_{0.0f, 0.0f, 0.0f}
	{}

	inline pointLight::pointLight(float ambientR, float ambientG, float ambientB, 
		float diffuseR, float diffuseG, float diffuseB,
		float specularR, float specularG, float specularB,
		float maxDistance)
	: light(ambientR, ambientG, ambientB, diffuseR, diffuseG, diffuseB, specularR, specularG, specularB),
	  maxDistance_(maxDistance), padding_{ 0.0f, 0.0f, 0.0f }
	{}

	inline unsigned int pointLight::nArgs() {

		return light::nArgs() + nArgs_;

	}

}

#endif