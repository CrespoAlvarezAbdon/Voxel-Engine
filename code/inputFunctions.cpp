#include "inputFunctions.h"



namespace VoxelEng {

	// 'inputFunctions' class.

	bool inputFunctions::initialised_ = false;
	camera* inputFunctions::playerCamera_ = nullptr;


	void inputFunctions::init() {
	
		if (initialised_)
			logger::errorLog("The input functions system is already initialised");
		else {
		
			playerCamera_ = &game::playerCamera();
			initialised_ = true;
		
		}
		
	}

	void inputFunctions::cleanUp() {
	
		playerCamera_ = nullptr;

		initialised_ = false;
	
	}

}