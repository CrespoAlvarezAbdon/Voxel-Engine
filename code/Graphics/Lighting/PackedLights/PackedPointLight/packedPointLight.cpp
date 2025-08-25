#include "packedPointLight.hpp"

namespace VoxelEng {

	packedPointLight packedPointLight::pack(const pointLight& l) {

		packedLight pl = packedLight::pack(l);
		return packedPointLight(pl.params[0], pl.params[1], pl.params[2], l.maxDistance());

	}

}