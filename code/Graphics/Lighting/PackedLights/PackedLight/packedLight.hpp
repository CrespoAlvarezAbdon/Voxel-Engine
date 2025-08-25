#ifndef _VOXELENG_PACKED_LIGHT_
#define _VOXELENG_PACKED_LIGHT_

#include <Graphics/Lighting/Lights/light.hpp>
#include <Registry/registryElement.h>

namespace VoxelEng {

	struct alignas(16) packedLight : public registryElement {

		static packedLight pack(const light& l);

		packedLight();

		packedLight(int ambient, int diffuse, int specular);

		int params[4]; // ambient, diffuse, specular, unused (can be used by subclasses).

	};

	inline packedLight::packedLight() 
	{
		std::memset(params, 0, sizeof(int)*4);
	}

}

#endif