#include "registries.h"

#include <memory>
#include "../logger.h"

namespace VoxelEng {

	bool registries::initialised_ = false;
	registryInsOrdered<std::string, material>* registries::materials_ = nullptr;

	void registries::init() {
	
		if (initialised_) {
		
			logger::errorLog("Registries collection already initialised");
		
		}
		else {
		
			// Note. Always add a "Default" element into registries in case it is wanted to be used in case
			// the specified registry element is not found.

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