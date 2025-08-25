#ifndef _VOXELENG_PACKED_SPOT_LIGHT_
#define _VOXELENG_PACKED_SPOT_LIGHT_

#include <Graphics/Lighting/Lights/SpotLight/spotLight.h>
#include <Graphics/Lighting/PackedLights/PackedLight/packedLight.hpp>

namespace VoxelEng {

	struct packedSpotLight : public packedLight {

		static packedSpotLight pack(const spotLight& l);

		packedSpotLight();

		packedSpotLight(int ambient, int diffuse, int specular, float cutOffAngle, float outerCutOffAngle, unsigned int maxDistance);

		float cutOffAngle_;
		float outerCutOffAngle_;
		unsigned int maxDistance_;
		float padding_; // Only used for padding.

	};

	inline packedSpotLight::packedSpotLight()
	: packedLight(), cutOffAngle_(0.0f), outerCutOffAngle_(0.0f), maxDistance_(0), padding_(0.0f)
	{}

	inline packedSpotLight::packedSpotLight(int ambient, int diffuse, int specular, float cutOffAngle,
		float outerCutOffAngle, unsigned int maxDistance)
	: packedLight(ambient, diffuse, specular), cutOffAngle_(cutOffAngle), 
		outerCutOffAngle_(outerCutOffAngle), maxDistance_(maxDistance), padding_(0.0f)
	{}

}

#endif