#ifndef _VOXELENG_PACKED_POINT_LIGHT_
#define _VOXELENG_PACKED_POINT_LIGHT_

#include <Graphics/Lighting/Lights/PointLight/pointLight.h>
#include <Graphics/Lighting/PackedLights/PackedLight/packedLight.hpp>

namespace VoxelEng {

	struct alignas(16) packedPointLight : public packedLight {

		static packedPointLight pack(const pointLight& l);

		packedPointLight();

		packedPointLight(int ambient, int diffuse, int specular, unsigned int maxDistance);

		// int params[4]; // defined in packedLight class: ambient, diffuse, specular, unused (used by this class to represent max distance).

	};

	inline packedPointLight::packedPointLight()
	: packedLight()
	{}

	inline packedPointLight::packedPointLight(int ambient, int diffuse, int specular, unsigned int maxDistance)
	: packedLight(ambient, diffuse, specular)
	{
		params[3] = maxDistance;
	}

}

#endif