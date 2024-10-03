#include "registry.h"

namespace VoxelEng {

	template <typename KeyT, typename T>
	requires std::derived_from<T, registryElement> 
	void registry<KeyT, T>::init(const std::string& typeName) {

		if (initialised_) {

			logger::errorLog("Registry element system is already initialised");

		}
		else {

			typeName_ = typeName;

			initialised_ = true;

		}

	}

}