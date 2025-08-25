#include "packedDirectionalLight.hpp"

namespace VoxelEng {

	packedDirectionalLight packedDirectionalLight::pack(const directionalLight& l) {

		packedLight pl = packedLight::pack(l);
		return packedDirectionalLight(pl.params[0], pl.params[1], pl.params[2]);

	}

}