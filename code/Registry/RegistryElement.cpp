#include "registryElement.h"
#include <Utilities/Logger/logger.h>

namespace VoxelEng {

	bool registryElement::initialised_ = false;
	std::string registryElement::typeName_ = "";

	void registryElement::init(const std::string& typeName) {

		if (initialised_)
			logger::errorLog("Registry element system is already initialised");
		else {
		
			typeName_ = typeName;

			initialised_ = true;
		
		}

	}

}