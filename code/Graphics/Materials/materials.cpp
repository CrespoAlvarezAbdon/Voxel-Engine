#include "materials.h"
#include <Utilities/Logger/logger.h>

namespace VoxelEng {

	bool material::initialised_ = false;
	std::string material::typeName_ = "";

	void material::init(const std::string& typeName) {

		if (initialised_)
			logger::errorLog("Material system is already initialised");
		else {

			typeName_ = typeName;

			initialised_ = true;

		}

	}

}