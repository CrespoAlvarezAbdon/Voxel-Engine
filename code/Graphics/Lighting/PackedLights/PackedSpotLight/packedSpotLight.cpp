#include "packedSpotLight.hpp"

namespace VoxelEng {

	packedSpotLight packedSpotLight::pack(const spotLight& l) {

		packedLight pl = packedLight::pack(l);
		return packedSpotLight(pl.params[0], pl.params[1], pl.params[2], l.cutOffAngle(), l.outerCutOffAngle(), l.maxDistance());

	}

}