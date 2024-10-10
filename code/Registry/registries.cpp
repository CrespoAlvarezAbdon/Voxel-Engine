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

namespace VoxelEng {

	bool registries::initialised_ = false;
	bool registries::graphicalModeInitialised_ = false;
	registry<std::string, var>* registries::registries_ = nullptr;
	registry<std::string, var>* registries::registriesInsOrdered_ = nullptr;

	void registries::init() {
	
		if (initialised_) {
		
			logger::errorLog("Registries collection already initialised");
		
		}
		else {
		
			// TODO. MOVE THE FACTORY FUNCTIONS TO A SEPARATE PART OF THE CODE TO KEEP THIS CONSTRUCTOR SIMPLE AND CLEAN.

			// Note. Always add a "Default" element into registries in case it is wanted to be used in case
			// the specified registry element is not found.

			// NEXT. ERA HACER UN REGISTRO DE VARS NO LO QUE HICIMOS EL DOMINGO. VUELVE A PASARLO TODO BIEN.

			// Registries of registries initialization.
			registries_ = new registry<std::string, var>([](std::any args) {

				auto tuple = std::any_cast<std::tuple<void*, var::varType>>(args);
				return std::make_unique<var>(std::get<0>(tuple), std::get<1>(tuple));

			}, nullptr);

			registriesInsOrdered_ = new registry<std::string, var>([](std::any args) {

				auto tuple = std::any_cast<std::tuple<void*, var::varType>>(args);
				return std::make_unique<var>(std::get<0>(tuple), std::get<1>(tuple));

			}, nullptr);

			// Material types registry initialisation.
			// IMPORTANT. REMEMBER THAT TEMPLATES DO NOT HANDLE THINGS LIKE TYPE INHERITANCE. REGISTRIESINSORDERED_'S INSERT FUNCTION EXPECTS A TUPLE OF VOID* AND VAR::VARTYPE AND NOTHING ELSE
			registriesInsOrdered_->insert("Materials", static_cast<void*>(new registryInsOrdered<std::string, material>([](std::any args) {

				auto tuple = std::any_cast<std::tuple<float, float, float, float, float, float, float, float, float, float>>(args);
				return std::make_unique<material>(std::get<0>(tuple), std::get<1>(tuple), std::get<2>(tuple),
					std::get<3>(tuple), std::get<4>(tuple), std::get<5>(tuple),
					std::get<6>(tuple), std::get<7>(tuple), std::get<8>(tuple),
					std::get<9>(tuple));

			}, nullptr)), var::varType::REGISTRYINSORDERED_OF_STRINGS_MATERIALS);

			registriesInsOrdered_->get("Materials")->pointer<registryInsOrdered<std::string, material>>()->insert("Default",
				1.0f, 1.0f, 1.0f,
				1.0f, 1.0f, 1.0f,
				1.0f, 1.0f, 1.0f,
				32.0f);

			// Light types registry initialisation.
			registriesInsOrdered_->insert("DirectionalLights", static_cast<void*>(new registryInsOrdered<std::string, directionalLight>([](std::any args) {

				auto tuple = std::any_cast<std::tuple<float, float, float, float, float, float>>(args);
				return std::make_unique<directionalLight>(std::get<0>(tuple), std::get<1>(tuple), std::get<2>(tuple),
					std::get<3>(tuple), std::get<4>(tuple), std::get<5>(tuple));

			}, nullptr)), var::varType::REGISTRYINSORDERED_OF_STRINGS_DIRECTIONALLIGHTS);

			registriesInsOrdered_->get("DirectionalLights")->pointer<registryInsOrdered<std::string, directionalLight>>()->insert("Default",
				1.0f, 1.0f, 1.0f, 
				1.0f, 1.0f, 1.0f);

			registriesInsOrdered_->insert("PointLights", static_cast<void*>(new registryInsOrdered<std::string, pointLight>([](std::any args) {

				auto tuple = std::any_cast<std::tuple<float, float, float, float, float, float, float, float, float>>(args);
				return std::make_unique<pointLight>(std::get<0>(tuple), std::get<1>(tuple), std::get<2>(tuple),
					std::get<3>(tuple), std::get<4>(tuple), std::get<5>(tuple),
					std::get<6>(tuple), std::get<7>(tuple), std::get<8>(tuple));

			}, nullptr)), var::varType::REGISTRYINSORDERED_OF_STRINGS_POINTLIGHTS);

			registriesInsOrdered_->get("PointLights")->pointer<registryInsOrdered<std::string, pointLight>>()->insert("Default",
				1.0f, 1.0f, 1.0f, 
				1.0f, 1.0f, 1.0f, 
				1.0f, 0.7f, 1.8f);

			registriesInsOrdered_->insert("SpotLights", static_cast<void*>(new registryInsOrdered<std::string, spotLight>([](std::any args) {

				auto tuple = std::any_cast<std::tuple<float, float, float, float, float, float, float, float>>(args);
				return std::make_unique<spotLight>(std::get<0>(tuple), std::get<1>(tuple), std::get<2>(tuple),
					std::get<3>(tuple), std::get<4>(tuple), std::get<5>(tuple),
					std::get<6>(tuple), std::get<7>(tuple));

			}, nullptr)), var::varType::REGISTRYINSORDERED_OF_STRINGS_SPOTLIGHTS);

			registriesInsOrdered_->get("SpotLights")->pointer<registryInsOrdered<std::string, spotLight>>()->insert("Default",
				1.0f, 1.0f, 1.0f,
				1.0f, 1.0f, 1.0f,
				25.0f, 35.0f);

			initialised_ = true;
		
		}
	
	}

	void registries::initGraphicalMode() {
	
		if (graphicalModeInitialised_)
			throw std::runtime_error("Registries graphical mode already initialised");
		else{

			if (initialised_) {

				// UBO registry initialisation.
				registries_->insert("UBOs", static_cast<void*>(new registry<std::string, var>([](std::any args) {

					auto tuple = std::any_cast<std::tuple<void*, var::varType>>(args);
					return std::make_unique<var>(std::get<0>(tuple), std::get<1>(tuple));

				}, nullptr)), var::varType::REGISTRY_OF_STRINGS_VARS);

				graphicalModeInitialised_ = true;

			}
			else
				throw std::runtime_error("Registries system must be initialised first before initialising registries graphical mode.");

		}
	
	}

	void registries::reset() {

		if (initialised_) {

			if (registries_) {

				delete registries_;
				registries_ = nullptr;

			}

			if (registriesInsOrdered_) {

				delete registriesInsOrdered_;
				registriesInsOrdered_ = nullptr;

			}

			initialised_ = false;

		}
		else
			logger::errorLog("Registries collection is not initialised");

	}

	void registries::resetGraphicalMode() {
	
		if (graphicalModeInitialised_) {

			if (initialised_) {

				registries_->erase("UBOs");

				graphicalModeInitialised_ = false;

			}
			else
				throw std::runtime_error("Registries system must be initialised before resetting registries graphical mode.");

		}
		else
			throw std::runtime_error("Registries graphical mode is not initialised");
	
	}

	var* registries::get(const std::string& name) {

		if (registries_->contains(name))
		{
			return registries_->get(name);
		}
		else
			throw std::runtime_error("The registry " + name + " is not registered");

	}

	var* registries::getInsOrdered(const std::string& name) {

		if (registriesInsOrdered_->contains(name))
		{
			return registriesInsOrdered_->get(name);
		}
		else
			throw std::runtime_error("The insertion-ordered registry " + name + " is not registered");

	}

}