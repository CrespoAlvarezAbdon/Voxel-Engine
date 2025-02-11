#include "var.h"
#include <Utilities/Logger/logger.h>

// Includes of the headers that define the types supported by the var class.
#include <Graphics/UBOs/UBOs.h>
#include <Graphics/SSBO/SSBO.h>
#include <Graphics/Lighting/Lights/DirectionalLight/directionalLight.h>
#include <Graphics/Lighting/Lights/PointLight/pointLight.h>
#include <Graphics/Lighting/Lights/SpotLight/spotLight.h>
#include <Graphics/Lighting/Lights/LightInstance/lightInstance.h>
#include <Graphics/Materials/materials.h>

namespace VoxelEng {

	bool var::initialised_ = false;
	std::string var::typeName_ = "";

	void var::init(const std::string& typeName) {

		if (initialised_)
			logger::errorLog("Registry element system is already initialised");
		else {

			typeName_ = typeName;

			initialised_ = true;

		}

	}

	var::~var() {
	
		switch (varType_) {
		
		case varType::NOTYPE:
			break;

		case varType::UBO_OF_MATERIALS:
			delete static_cast<UBO<material>*>(pointer_);
			break;

		case varType::UBO_OF_DIRECTIONALLIGHTS:
			delete static_cast<UBO<directionalLight>*>(pointer_);
			break;

		case varType::UBO_OF_POINTLIGHTS:
			delete static_cast<UBO<pointLight>*>(pointer_);
			break;

		case varType::UBO_OF_SPOTLIGHTS:
			delete static_cast<UBO<spotLight>*>(pointer_);
			break;

		case varType::SSBO_OF_LIGHTINSTANCES:
			delete static_cast<SSBO<lightInstance>*>(pointer_);
			break;

		case varType::REGISTRYINSORDERED_OF_STRINGS_MATERIALS:
			delete static_cast<registry<std::string, material>*>(pointer_);
			break;

		case varType::REGISTRYINSORDERED_OF_STRINGS_DIRECTIONALLIGHTS:
			delete static_cast<registry<std::string, directionalLight>*>(pointer_);
			break;

		case varType::REGISTRYINSORDERED_OF_STRINGS_POINTLIGHTS:
			delete static_cast<registry<std::string, pointLight>*>(pointer_);
			break;

		case varType::REGISTRYINSORDERED_OF_STRINGS_SPOTLIGHTS:
			delete static_cast<registry<std::string, spotLight>*>(pointer_);
			break;

		case varType::REGISTRY_OF_STRINGS_VARS:
			delete static_cast<registry<std::string, var>*>(pointer_);
			break;

		case varType::DIRECTIONALLIGHT:
			delete static_cast<directionalLight*>(pointer_);
			break;

		case varType::POINTLIGHT:
			delete static_cast<pointLight*>(pointer_);
			break;

		case varType::SPOTLIGHT:
			delete static_cast<spotLight*>(pointer_);
			break;

		default:
		case varType::UNKNOWN:
			logger::errorLog("Unsupported var type specified");
			break;
		
		}
		pointer_ = nullptr;

	}

}