#include "pointLight.h"

#include <Utilities/Logger/logger.h>

namespace VoxelEng {

	const unsigned int pointLight::nArgs_ = 1; // Always the number of arguments added by THIS CLASS ONLY.

	lightIntensity pointLight::intensity(colorChannel channel) const {

		lightIntensity intensity = 0;
		switch (channel) {

		case colorChannel::RED:
			intensity = intensity_.x;
			break;

		case colorChannel::GREEN:
			intensity = intensity_.y;
			break;

		case colorChannel::BLUE:
			intensity = intensity_.z;
			break;

		case colorChannel::ALPHA:
			intensity = intensity_.w;
			break;

		default:
			logger::errorLog("Unsupported color channel " + static_cast<int>(channel));
			break;

		}
		return intensity;

	}

}