#ifndef _VOXENG_UTILITIES_
#define _VOXENG_UTILITIES_

namespace VoxelEng {

	/*
	Computes a % b using floor modulo operations.
	The equation computed is
	r = a - b * q
	where
	q = (int)floor((double)a / b)
	*/
	int floorMod(int a, int b);

}

#endif