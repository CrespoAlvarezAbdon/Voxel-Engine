#include "registries.h"

#include <memory>
#include <initializer_list>
#include <stdexcept>
#include "../logger.h"

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

				auto tuple = std::any_cast<std::tuple<std::string, std::initializer_list<float>>>(args);
				const std::string& type = std::get<0>(tuple);
				const std::initializer_list<float>& arguments = std::get<1>(tuple);
				
				if (type == "light")
					throw std::runtime_error("It is not supported to register lights of the light base class.");
				else if (type == "directionalLight") {
				
					if (arguments.size() == 0) {



					}
					else
						throw std::runtime_error("For directional lights, the number of arguments must be " + );
				
				}
				else if (type == "pointLight") {
				
				
				
				}
				else if (type == "spotLight") {
				
				
				
				}

			}, nullptr);

			initialised_ = true;
		
		}
	
	}

	void registries::deinit() {

		if (initialised_) {

			if (materials_)	{

				delete materials_;
				materials_ = nullptr;

			}

			initialised_ = false;
			
		}
		else {

			logger::errorLog("Registries collection is not initialised");

		}

	}

}