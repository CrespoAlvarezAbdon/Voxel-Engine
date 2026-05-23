#ifndef _VOXELENG_POINT_LIGHT_
#define _VOXELENG_POINT_LIGHT_

#include <definitions.h>
#include <vec.h>
#include <Registry/registryElement.h>
#include <Graphics/Lighting/definitions.hpp>
#include <Graphics/Lighting/lights/light.hpp>

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
		pointLight(lightValue ambientR, lightValue ambientG, lightValue ambientB, lightValue ambientA,
			lightValue diffuseR, lightValue diffuseG, lightValue diffuseB, lightValue diffuseA,
			lightValue specularR, lightValue specularG, lightValue specularB, lightValue specularA,
			lightIntensity intensityR, lightIntensity intensityG, lightIntensity intensityB, lightIntensity intensityA);


		// Observers.

		/**
		* @brief Get the number of arguments needed to construct an object of this class.
		* @returns The number of arguments needed to construct an object of this class.
		*/
		static unsigned int nArgs();

		/**
		* @brief Get the light's maximum intensity.
		* @returns The light's maximum intensity.
		*/
		const basicUVec4& intensity() const;

		/**
		* @brief Get the light's maximum intensity.
		* Value range is [0, 8].
		* @param channel The channel form which to get the light's maximum intensity (only one channel at a time).
		* @returns The light's maximum intensity.
		*/
		lightIntensity intensity(colorChannel channel) const;

	protected:

		static const unsigned int nArgs_;

		basicUVec4 intensity_;
		
	};

	inline pointLight::pointLight()
	: light(), intensity_(basicUVec4Zero)
	{}

	inline pointLight::pointLight(lightValue ambientR, lightValue ambientG, lightValue ambientB, lightValue ambientA,
		lightValue diffuseR, lightValue diffuseG, lightValue diffuseB, lightValue diffuseA,
		lightValue specularR, lightValue specularG, lightValue specularB, lightValue specularA,
		lightIntensity intensityR, lightIntensity intensityG, lightIntensity intensityB, lightIntensity intensityA)
	: light(ambientR, ambientG, ambientB, ambientA, 
		diffuseR, diffuseG, diffuseB, diffuseA, 
		specularR, specularG, specularB, specularA),
	  intensity_(intensityR, intensityG, intensityB, intensityA)
	{}

	inline unsigned int pointLight::nArgs() {

		return light::nArgs() + nArgs_;

	}

	inline const basicUVec4& pointLight::intensity() const {
	
		return intensity_;
	
	}

}

#endif