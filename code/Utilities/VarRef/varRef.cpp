#include "varRef.h"
#include <Utilities/Logger/logger.h>

namespace VoxelEng {

	bool varRef::initialised_ = false;
	std::string varRef::typeName_ = "";

	void varRef::init(const std::string& typeName) {

		if (initialised_)
			logger::errorLog("Registry element system is already initialised");
		else {

			typeName_ = typeName;

			initialised_ = true;

		}

	}

}