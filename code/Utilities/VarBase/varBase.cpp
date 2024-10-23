#include "varBase.h"
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

	bool varBase::initialised_ = false;
	std::string varBase::typeName_ = "";

	void varBase::init(const std::string& typeName) {

		if (initialised_)
			logger::errorLog("Registry element system is already initialised");
		else {

			typeName_ = typeName;

			initialised_ = true;

		}

	}

}