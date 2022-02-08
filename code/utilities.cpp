#include "utilities.h"
#include <cmath>

#include <iostream>
#include <ostream>

namespace VoxelEng {

	
	int floorMod(int a, int b) {
	
		return a - b * (int)(std::floor((double)a / b));
	
	}

	int indMaxAbsVec(const glm::vec3 vec) {
	
		float x = abs(vec.x),
			  y = abs(vec.y),
			  z = abs(vec.z);

		if (x >= y)
			return x >= z ? 0 : 2;
		else
			return y >= z ? 1 : 2;
	
	}

}