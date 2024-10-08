#include "timer.h"
#include <Utilities/Logger/logger.h>

namespace VoxelEng {

	timer::timer()
	: hasStarted_(false), hasDuration_(false) {}

	void timer::start() {

		if (hasStarted_)
			logger::errorLog("A previous called to start() was made and finish() was not called");
		else {
		
			tStart_ = std::chrono::high_resolution_clock::now();
			hasStarted_ = true;
		
		}

	}

	void timer::finish() {

		if (hasStarted_) {
		
			tEnd_ = std::chrono::high_resolution_clock::now();
			hasStarted_ = false;
			hasDuration_ = true;
		
		}
		else
			logger::errorLog("No call to start() without a paired one to finish() was made before this call");
	
	}

	duration timer::getDurationMs() {
	
		if (hasStarted_)
			logger::errorLog("No call to finish() was made after the call to start()");
		else
		{

			if (hasDuration_)
				return std::chrono::duration_cast<std::chrono::milliseconds>(tEnd_ - tStart_).count();
			else
				logger::errorLog("The timer has not been used only once and because of this it doesn not have any duration to return");

		}
	
	}

}