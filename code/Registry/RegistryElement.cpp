#include "RegistryElement.h"


namespace VoxelEng {

	const bool registryElement::initialized_ = false;
	std::string registryElement::typeName_ = "";

	void registryElement::initialize(const std::string& typeName) {
	
		typeName_ = typeName;
	
	}

	const std::string& registryElement::typeName() {
	
		return typeName_;
	
	}

}