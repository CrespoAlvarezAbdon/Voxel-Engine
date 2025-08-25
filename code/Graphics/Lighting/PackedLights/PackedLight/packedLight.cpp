#include "packedLight.hpp"

namespace VoxelEng {

	packedLight packedLight::pack(const light& l) {
	
		const basicVec4& ambient = l.ambient();
		const basicVec4& diffuse = l.diffuse();
		const basicVec4& specular = l.specular();
		return packedLight(
			(uint32_t)ambient.x |
			((uint32_t)ambient.y << 8) |
			((uint32_t)ambient.z << 16) |
			((uint32_t)ambient.w << 24),
			(uint32_t)diffuse.x |
			((uint32_t)diffuse.y << 8) |
			((uint32_t)diffuse.z << 16) |
			((uint32_t)diffuse.w << 24),
			(uint32_t)specular.x |
			((uint32_t)specular.y << 8) |
			((uint32_t)specular.z << 16) |
			((uint32_t)specular.w << 24)
		);
	
	}

	packedLight::packedLight(int ambient, int diffuse, int specular)
	{
		params[0] = ambient;
		params[1] = diffuse;
		params[2] = specular;
		params[3] = 0;
	}

}