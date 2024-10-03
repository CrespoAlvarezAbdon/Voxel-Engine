#include "var.h"
#include <Utilities/Logger/logger.h>

// Includes of the headers that define the types supported by the var class.
#include <Graphics/UBOs/UBOs.h>
#include <Graphics/Materials/materials.h>

namespace VoxelEng {

	bool var::initialised_ = false;
	std::string var::typeName_ = "";

	void var::init(const std::string& typeName) {

		if (initialised_) {

			logger::errorLog("Registry element system is already initialised");

		}
		else {

			typeName_ = typeName;

			initialised_ = true;

		}

	}

	void var::destroy() {
	
		switch (varType_) {
		
		case varType::UBO_OF_MATERIALS:
			delete static_cast<UBO<material>*>(pointer_);
			break;

		case varType::UNKNOWN:
			logger::errorLog("Unsupported var type specified");
			break;
		
		}
	
	}

}