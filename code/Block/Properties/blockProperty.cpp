#include "blockProperty.hpp"

#include <Utilities/Logger/logger.h>

namespace VoxelEng {

	bool blockProperty::initialised_ = false;
	blockProperty* blockProperty::emptyProp_ = nullptr;

	void blockProperty::init() {

		if (initialised_)
			logger::errorLog("Block system is already initialised");
		else {

			emptyProp_ = new blockProperty();

			initialised_ = true;

		}
	}

	void blockProperty::reset() {

		if (initialised_) {

			delete emptyProp_;
			emptyProp_ = nullptr;

			initialised_ = false;

		}
		else
			logger::errorLog("Block system is not initialised");

	}

}