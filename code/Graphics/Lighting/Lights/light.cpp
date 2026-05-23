#include "light.hpp"
#include <Utilities/Logger/logger.h>

namespace VoxelEng {

	bool light::initialised_ = false;
	std::string light::typeName_ = "";
	const unsigned int light::nArgs_ = 9;

	void light::init(const std::string& typeName) {

		if (initialised_)
			logger::errorLog("Light registry element system is already initialised");
		else {

			typeName_ = typeName;

			initialised_ = true;

		}

	}

	lightValue light::ambient(colorChannel channel) const {

		lightValue value = 0;
		switch (channel) {

		case colorChannel::RED:
			value = ambient_.x;
			break;

		case colorChannel::GREEN:
			value = ambient_.y;
			break;

		case colorChannel::BLUE:
			value = ambient_.z;
			break;

		case colorChannel::ALPHA:
			value = ambient_.w;
			break;

		default:
			logger::errorLog("Unsupported color channel " + static_cast<int>(channel));
			break;

		}
		return value;

	}

}