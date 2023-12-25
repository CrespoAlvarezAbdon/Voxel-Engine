#include "inputFunctions.h"


namespace VoxelEng {

	// 'inputFunctions' class.

	bool inputFunctions::initialised_ = false;


	void inputFunctions::init() {
	
		if (initialised_)
			logger::errorLog("The input functions system is already initialised");
		else {
		
			initialised_ = true;
		
		}
		
	}

	void inputFunctions::reset() {
	
		initialised_ = false;
	
	}

}