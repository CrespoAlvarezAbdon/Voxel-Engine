#ifndef _VOXELENG_PACKING_
#define _VOXELENG_PACKING_

#include <cstdint>
#include <vector>

#include <definitions.h>

namespace VoxelEng {

	/**
	* @brief Encode the given normal vector into a OpenGL's GL_INT_2_10_10_10_REV 32-bit unsigned integer.
	*/
	uint32_t packNormalIntoGL_INT_2_10_10_10_REV(float normalX, float normalY, float normalZ);

}

#endif