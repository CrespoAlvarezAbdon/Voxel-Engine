#ifndef _VOXENG_UTILITIES_
#define _VOXENG_UTILITIES_
#include <glm.hpp>

namespace VoxelEng {

	/*
	Computes a % b using floor modulo operations.
	The equation computed is
	r = a - b * q
	where
	q = (int)floor((double)a / b)
	*/
	int floorMod(int a, int b);


	/*
	Returns +1 if 'real' >= 0 and -1 otherwise.
	*/
	inline int sign(float real) {
	
		return real >= 0 ? 1 : -1;
	
	};

}

#endif