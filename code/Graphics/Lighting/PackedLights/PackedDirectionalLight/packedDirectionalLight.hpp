#ifndef _VOXELENG_PACKED_DIRECTIONAL_LIGHT_
#define _VOXELENG_PACKED_DIRECTIONAL_LIGHT_

#include <Graphics/Lighting/Lights/DirectionalLight/directionalLight.h>
#include <Graphics/Lighting/PackedLights/PackedLight/packedLight.hpp>

namespace VoxelEng {

	struct alignas(16) packedDirectionalLight : public packedLight {
		
		static packedDirectionalLight pack(const directionalLight& l);

		packedDirectionalLight();

		packedDirectionalLight(int ambient, int diffuse, int specular);

	};

	inline packedDirectionalLight::packedDirectionalLight() 
	: packedLight()
	{}

	inline packedDirectionalLight::packedDirectionalLight(int ambient, int diffuse, int specular)
	: packedLight(ambient, diffuse, specular)
	{}

}

#endif