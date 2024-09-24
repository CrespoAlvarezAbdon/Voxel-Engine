#ifndef _VOXELENG_POINT_LIGHT_
#define _VOXELENG_POINT_LIGHT_

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
		* @param diffuseR Percentage of red color emitted in diffuse lighting calculations by this light type.
		* @param diffuseG Percentage of green color emitted in diffuse lighting calculations by this light type.
		* @param diffuseB Percentage of blue color emitted in diffuse lighting calculations by this light type.
		* @param specularR Percentage of red color emitted in specular lighting calculations by this light type.
		* @param specularG Percentage of green color emitted in specular lighting calculations by this light type.
		* @param specularB Percentage of blue color emitted in specular lighting calculations by this light type.
		* @param constant Commonly equal to 1.0 to ensure the denominator used in the point light lighting formula never gets smaller than 1 in order to avoid
		* boosting the instenity with specific distances.
		* @param linear Linear term in the point light lighting formula that reduces the light's intensity. It is multiplied with the distance to the light.
		* @param quadratic Quadratic term in the point light lighting formula that reduces the light's intensity. It is multiplied with the distance to the light.
		*/
		pointLight(float diffuseR, float diffuseG, float diffuseB,
			float specularR, float specularG, float specularB,
			float constant, float linear, float quadratic);


		// Observers.

		/**
		* @brief Get the number of arguments needed to construct an object of this class.
		* @returns The number of arguments needed to construct an object of this class.
		*/
		static unsigned int nArgs();

	protected:

		static const unsigned int nArgs_;

		float constant_;
		float linear_;
		float quadratic_;
		
	};

	inline pointLight::pointLight()
	: light(),
		constant_(1.0f), linear_(1.0f), quadratic_(1.0f)
	{}

	inline pointLight::pointLight(float diffuseR, float diffuseG, float diffuseB,
		float specularR, float specularG, float specularB,
		float constant, float linear, float quadratic)
	: light(diffuseR, diffuseG, diffuseB, specularR, specularG, specularB), 
		constant_(constant), linear_(linear), quadratic_(quadratic)
	{}

	inline unsigned int pointLight::nArgs() {

		return light::nArgs() + nArgs_;

	}

}

#endif