#ifndef _VOXELENG_SPOT_LIGHT_
#define _VOXELENG_SPOT_LIGHT_

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
		* @param diffuseR Percentage of red color emitted in diffuse lighting calculations by this light type.
		* @param diffuseG Percentage of green color emitted in diffuse lighting calculations by this light type.
		* @param diffuseB Percentage of blue color emitted in diffuse lighting calculations by this light type.
		* @param specularR Percentage of red color emitted in specular lighting calculations by this light type.
		* @param specularG Percentage of green color emitted in specular lighting calculations by this light type.
		* @param specularB Percentage of blue color emitted in specular lighting calculations by this light type.
		* @param cutOffAngle Everything outside this angle is not lit by the spotlight.
		* @param outerCutOffAngle 
		*/
		spotLight(float diffuseR, float diffuseG, float diffuseB,
			float specularR, float specularG, float specularB,
			float cutOffAngle, float outerCutOffAngle);

		// NEXT. METER CONSTRUCTORES DE POINT Y SPOT LIGHT Y VER QUÉ MÁS HAY QUE PONER EN GRAPHICS.CPP PARA TENER ESTO READY.


		// Observers.

		/**
		* @brief Get the number of arguments needed to construct an object of this class.
		* @returns The number of arguments needed to construct an object of this class.
		*/
		static unsigned int nArgs();

	protected:

		static const unsigned int nArgs_;

		float cutOffAngle_;
		float outerCutOffAngle_;
		float padding_[2]; // Only used for padding.
		
	};

	inline spotLight::spotLight()
		: directionalLight(), cutOffAngle_(90.0f), outerCutOffAngle_(45.0f), padding_{0.0f, 0.0f}
	{}

	inline spotLight::spotLight(float diffuseR, float diffuseG, float diffuseB,
		float specularR, float specularG, float specularB,
		float cutOffAngle, float outerCutOffAngle)
	: directionalLight(diffuseR, diffuseG, diffuseB, specularR, specularG, specularB),
		cutOffAngle_(cutOffAngle), outerCutOffAngle_(outerCutOffAngle), padding_{ 0.0f, 0.0f }
	{}

	inline unsigned int spotLight::nArgs() {

		return directionalLight::nArgs() + nArgs_;

	}

}

#endif