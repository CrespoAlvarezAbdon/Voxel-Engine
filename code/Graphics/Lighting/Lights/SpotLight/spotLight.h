#ifndef _VOXELENG_SPOT_LIGHT_
#define _VOXELENG_SPOT_LIGHT_

#include <definitions.h>
#include <vec.h>
#include <Registry/registryElement.h>
#include <Graphics/Lighting/lights/DirectionalLight/directionalLight.h>

namespace VoxelEng {

	class spotLight : public directionalLight {

	public:

		// Constructors.

		/**
		* @brief Default class constructor.
		*/
		spotLight();

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
		* @param cutOffAngle Everything outside this angle is not lit by the spotlight.
		* @param outerCutOffAngle TODO.
		* @param maxDistance Maximum distance this light can cover.
		*/
		spotLight(char ambientR, char ambientG, char ambientB,
			char diffuseR, char diffuseG, char diffuseB,
			char specularR, char specularG, char specularB,
			float cutOffAngle, float outerCutOffAngle, unsigned int maxDistance);


		// Observers.

		/**
		* @brief Get the number of arguments needed to construct an object of this class.
		* @returns The number of arguments needed to construct an object of this class.
		*/
		static unsigned int nArgs();

		float cutOffAngle() const;

		float outerCutOffAngle() const;

		unsigned int maxDistance() const;

	protected:

		static const unsigned int nArgs_;

		float cutOffAngle_;
		float outerCutOffAngle_;
		unsigned int maxDistance_;
		float padding_; // Only used for padding.
		
	};

	inline spotLight::spotLight()
		: directionalLight(), cutOffAngle_(90.0f), outerCutOffAngle_(45.0f), maxDistance_(lightMaxIntensity), padding_{0.0f}
	{}

	inline spotLight::spotLight(char ambientR, char ambientG, char ambientB,
		char diffuseR, char diffuseG, char diffuseB,
		char specularR, char specularG, char specularB,
		float cutOffAngle, float outerCutOffAngle, unsigned int maxDistance)
	: directionalLight(ambientR, ambientG, ambientB, diffuseR, diffuseG, diffuseB, specularR, specularG, specularB),
		cutOffAngle_(cutOffAngle), outerCutOffAngle_(outerCutOffAngle), maxDistance_(maxDistance), padding_{ 0.0f }
	{}

	inline unsigned int spotLight::nArgs() {

		return directionalLight::nArgs() + nArgs_;

	}

	inline float spotLight::cutOffAngle() const {
	
		return cutOffAngle_;
	
	}

	inline float spotLight::outerCutOffAngle() const {
	
		return outerCutOffAngle_;
	
	}

	inline unsigned int spotLight::maxDistance() const {
	
		return maxDistance_;
	
	}

}

#endif