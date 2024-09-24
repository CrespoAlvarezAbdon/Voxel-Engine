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
	registryInsOrdered<std::string, material>* registries::materials_ = nullptr;
	registry<std::string, light>* registries::lights_ = nullptr;

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
			lights_ = new registry<std::string, light>([](std::any args) {

				auto tuple = std::any_cast<std::tuple<std::string, std::vector<float>>>(args);
				const std::string& type = std::get<0>(tuple);
				const std::vector<float>& arguments = std::get<1>(tuple);
				std::size_t nArgs = arguments.size();

				if (type == "light")
					throw std::runtime_error("It is not supported to register lights of the light base class.");
				else if (type == "directionalLight") {

					if (nArgs == directionalLight::nArgs()) {

						directionalLight* l = new directionalLight(arguments[0], arguments[1], arguments[2], arguments[3], arguments[4], arguments[5]);
						return std::unique_ptr<light>(l);

					}
					else
						throw std::runtime_error("For directional lights, the number of arguments must be " + std::to_string(directionalLight::nArgs()) +
							"but " + std::to_string(nArgs) + " were provided");

				}
				else if (type == "pointLight") {

					if (nArgs == pointLight::nArgs()) {

						pointLight* l = new pointLight(arguments[0], arguments[1], arguments[2], arguments[3], arguments[4], arguments[5],
							arguments[6], arguments[7], arguments[8]);
						return std::unique_ptr<light>(l);

					}
					else
						throw std::runtime_error("For point lights, the number of arguments must be " + std::to_string(pointLight::nArgs()) +
							"but " + std::to_string(nArgs) + " were provided");

				}
				else if (type == "spotLight") {

					if (nArgs == spotLight::nArgs()) {

						spotLight* l = new spotLight(arguments[0], arguments[1], arguments[2], arguments[3], arguments[4], arguments[5],
							arguments[6], arguments[7]);
						return std::unique_ptr<light>(l);

					}
					else
						throw std::runtime_error("For spot lights, the number of arguments must be " + std::to_string(spotLight::nArgs()) +
							"but " + std::to_string(nArgs) + " were provided");

				}
				else
					throw std::runtime_error("The type of light " + type + "is not supported");

			}, nullptr);

			lights_->insert("DefaultDirectionalLight", "directionalLight", 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f);
			lights_->insert("DefaultPointLight", "pointLight", 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.7f, 1.8f);
			lights_->insert("DefaultSpotLight", "spotLight", 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 25.0f, 35.0f);

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