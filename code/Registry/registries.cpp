#include "registries.h"
#include <cstddef>
#include <memory>
#include <initializer_list>
#include <Graphics/Materials/materials.h>
#include <Graphics/Lighting/Lights/DirectionalLight/directionalLight.h>
#include <Graphics/Lighting/Lights/PointLight/pointLight.h>
#include <Graphics/Lighting/Lights/SpotLight/spotLight.h>
#include <Graphics/UBOs/UBOs.h>
#include <Utilities/Logger/logger.h>
#include <Utilities/Var/var.h>

namespace VoxelEng {

	bool registries::initialised_ = false;
	registry<std::string, registry<std::string, registryElement>>* registries::registries_ = nullptr;
	registry<std::string, registryInsOrdered<std::string, registryElement>>* registries::registriesInsOrdered_ = nullptr;

	void registries::init() {
	
		if (initialised_) {
		
			logger::errorLog("Registries collection already initialised");
		
		}
		else {
		
			// TODO. MOVE THE FACTORY FUNCTIONS TO A SEPARATE PART OF THE CODE TO KEEP THIS CONSTRUCTOR SIMPLE AND CLEAN.

			// Note. Always add a "Default" element into registries in case it is wanted to be used in case
			// the specified registry element is not found.

			//Registries of registries initialization.
			registries_ = new registry<std::string, registry<std::string, registryElement>>([](std::any args) {

				auto tuple = std::any_cast<std::tuple<registry<std::string, registryElement>::factoryFunc,
													  registry<std::string, registryElement>::onInsertFunc>>(args);
				return std::make_unique<registry<std::string, registryElement>>(std::get<0>(tuple), std::get<1>(tuple));

			}, nullptr);

			registriesInsOrdered_ = new registryInsOrdered<std::string, registryInsOrdered<std::string, registryElement>>([](std::any args) {

				auto tuple = std::any_cast<std::tuple<registryInsOrdered<std::string, registryElement>::factoryFunc,
													  registryInsOrdered<std::string, registryElement>::onInsertFunc>>(args);
				return std::make_unique<registryInsOrdered<std::string, registryElement>>(std::get<0>(tuple), std::get<1>(tuple));

			}, nullptr);

			// Material types registry initialisation.
			registriesInsOrdered_->insert("Materials", [](std::any args) {

				auto tuple = std::any_cast<std::tuple<float, float, float, float, float, float, float, float, float, float>>(args);
				return std::make_unique<material>(std::get<0>(tuple), std::get<1>(tuple), std::get<2>(tuple),
					std::get<3>(tuple), std::get<4>(tuple), std::get<5>(tuple),
					std::get<6>(tuple), std::get<7>(tuple), std::get<8>(tuple),
					std::get<9>(tuple));

			}, nullptr);

			registriesInsOrdered_->get("Materials").insert("Default",
				1.0f, 1.0f, 1.0f,
				1.0f, 1.0f, 1.0f,
				1.0f, 1.0f, 1.0f,
				32.0f);

			// Light types registry initialisation.
			registriesInsOrdered_->insert("DirectionalLights", [](std::any args) {

				auto tuple = std::any_cast<std::tuple<float, float, float, float, float, float>>(args);
				return std::make_unique<directionalLight>(std::get<0>(tuple), std::get<1>(tuple), std::get<2>(tuple),
					std::get<3>(tuple), std::get<4>(tuple), std::get<5>(tuple));

			}, nullptr);

			registriesInsOrdered_->get("DirectionalLights").insert("Default",
				1.0f, 1.0f, 1.0f, 
				1.0f, 1.0f, 1.0f);

			registriesInsOrdered_->insert("PointLights", [](std::any args) {

				auto tuple = std::any_cast<std::tuple<float, float, float, float, float, float, float, float, float>>(args);
				return std::make_unique<pointLight>(std::get<0>(tuple), std::get<1>(tuple), std::get<2>(tuple),
					std::get<3>(tuple), std::get<4>(tuple), std::get<5>(tuple),
					std::get<6>(tuple), std::get<7>(tuple), std::get<8>(tuple));

			}, nullptr);

			registriesInsOrdered_->get("PointLights").insert("Default",
				1.0f, 1.0f, 1.0f, 
				1.0f, 1.0f, 1.0f, 
				1.0f, 0.7f, 1.8f);

			registriesInsOrdered_->insert("SpotLights", [](std::any args) {

				auto tuple = std::any_cast<std::tuple<float, float, float, float, float, float, float, float>>(args);
				return std::make_unique<spotLight>(std::get<0>(tuple), std::get<1>(tuple), std::get<2>(tuple),
					std::get<3>(tuple), std::get<4>(tuple), std::get<5>(tuple),
					std::get<6>(tuple), std::get<7>(tuple));

			}, nullptr);

			registriesInsOrdered_->get("SpotLights").insert("Default",
				1.0f, 1.0f, 1.0f,
				1.0f, 1.0f, 1.0f,
				25.0f, 35.0f);

			registries_->insert("UBOs", [](std::any args) {

				auto tuple = std::any_cast<std::tuple<void*, var::varType>>(args);
				return std::make_unique<var>(std::get<0>(tuple), std::get<1>(tuple));

			}, nullptr);

			registries_->get("UBOs").insert("Materials", new UBO<material>("Materials", registries::getInsOrdered<material>("Materials"), 1), var::varType::UBO_OF_MATERIALS);

			initialised_ = true;
		
		}
	
	}

	void registries::deinit() {

		if (initialised_) {

			initialised_ = false;
			
		}
		else {

			logger::errorLog("Registries collection is not initialised");

		}

	}

}