#include "registries.h"

#include <memory>
#include "../logger.h"

namespace VoxelEng {

	bool registries::initialised_ = false;
	registry<std::string, material>* registries::materials_ = nullptr;

	void registries::init() {
	
		if (initialised_) {
		
			logger::errorLog("Registries collection already initialised");
		
		}
		else {
		
			materials_ = new registry<std::string, material>([](std::any args) {
			
				auto tuple = std::any_cast<std::tuple<float, float, float, float, float, float, float, float, float, float>>(args);
				return std::make_unique<material>(std::get<0>(tuple), std::get<1>(tuple), std::get<2>(tuple), 
					std::get<3>(tuple), std::get<4>(tuple), std::get<5>(tuple),
					std::get<6>(tuple), std::get<7>(tuple), std::get<8>(tuple),
					std::get<9>(tuple));
			
			});

			initialised_ = true;
		
		}
	
	}

}