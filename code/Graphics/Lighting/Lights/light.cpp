#include "light.h"
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


}