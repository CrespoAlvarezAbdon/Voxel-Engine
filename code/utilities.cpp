#include "utilities.h"
#include <cmath>

namespace VoxelEng {

	
	int floorMod(int a, int b) {
	
		return a - b * (int)(std::floor((double)a / b));
	
	}

}