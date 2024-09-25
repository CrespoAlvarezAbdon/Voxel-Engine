#include "registries.h"

#include <cstddef>
#include <memory>
#include <initializer_list>
#include <stdexcept>
#include <logger.h>
#include <Graphics/Lighting/Lights/DirectionalLight/directionalLight.h>
#include <Graphics/Lighting/Lights/PointLight/pointLight.h>
#include <Graphics/Lighting/Lights/SpotLight/spotLight.h>

namespace VoxelEng {

	bool registries::initialised_ = false;
	registryInsOrdered<std::string, directionalLight>* registries::directionalLights_ = nullptr;
	registryInsOrdered<std::string, pointLight>* registries::pointLights_ = nullptr;
	registryInsOrdered<std::string, spotLight>* registries::spotLights_ = nullptr;

	void registries::init() {
	
		if (initialised_) {
		
			logger::errorLog("Registries collection already initialised");
		
		}
		else {
		
			// TODO. MOVE THE FACTORY FUNCTIONS TO A SEPARATE PART OF THE CODE TO KEEP THIS CONSTRUCTOR SIMPLE AND CLEAN.

			// Note. Always add a "Default" element into registries in case it is wanted to be used in case
			// the specified registry element is not found.

			// Material types registry initialisation.
			materials_ = new registryInsOrdered<std::string, material>([](std::any args) {
			
				auto tuple = std::any_cast<std::tuple<float, float, float, float, float, float, float, float, float, float>>(args);
				return std::make_unique<material>(std::get<0>(tuple), std::get<1>(tuple), std::get<2>(tuple), 
					std::get<3>(tuple), std::get<4>(tuple), std::get<5>(tuple),
					std::get<6>(tuple), std::get<7>(tuple), std::get<8>(tuple),
					std::get<9>(tuple));
			
			}, nullptr);

			materials_->insert("Default",
				1.0f, 1.0f, 1.0f,
				1.0f, 1.0f, 1.0f,
				1.0f, 1.0f, 1.0f,
				32.0f);

			// Light types registry initialisation.
			// NOTE. First argument must be the name of a class that is light class itself or one of its derived classes.
			directionalLights_ = new registryInsOrdered<std::string, directionalLight>([](std::any args) {

				auto tuple = std::any_cast<std::tuple<float, float, float, float, float, float>>(args);
				return std::make_unique<directionalLight>(std::get<0>(tuple), std::get<1>(tuple), std::get<2>(tuple),
					std::get<3>(tuple), std::get<4>(tuple), std::get<5>(tuple));

			}, nullptr);

			directionalLights_->insert("Default", 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f);

			pointLights_ = new registryInsOrdered<std::string, pointLight>([](std::any args) {

				auto tuple = std::any_cast<std::tuple<float, float, float, float, float, float, float, float, float>>(args);
				return std::make_unique<pointLight>(std::get<0>(tuple), std::get<1>(tuple), std::get<2>(tuple),
					std::get<3>(tuple), std::get<4>(tuple), std::get<5>(tuple),
					std::get<6>(tuple), std::get<7>(tuple), std::get<8>(tuple));

			}, nullptr);

			pointLights_->insert("Default", 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.7f, 1.8f);

			spotLights_ = new registryInsOrdered<std::string, spotLight>([](std::any args) {

				auto tuple = std::any_cast<std::tuple<float, float, float, float, float, float, float, float>>(args);
				return std::make_unique<spotLight>(std::get<0>(tuple), std::get<1>(tuple), std::get<2>(tuple),
					std::get<3>(tuple), std::get<4>(tuple), std::get<5>(tuple),
					std::get<6>(tuple), std::get<7>(tuple));

			}, nullptr);

			spotLights_->insert("Default", 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 25.0f, 35.0f);

			initialised_ = true;
		
		}
	
	}

	void registries::deinit() {

		if (initialised_) {

			if (materials_)	{

				delete materials_;
				materials_ = nullptr;

			}

			if (lights_) {

				delete lights_;
				lights_ = nullptr;

			}

			initialised_ = false;
			
		}
		else {

			logger::errorLog("Registries collection is not initialised");

		}

	}

}