#include "packing.h"

#include <stdexcept>

namespace VoxelEng {

	uint32_t packNormalIntoGL_INT_2_10_10_10_REV(float normalX, float normalY, float normalZ) {

		// Convert from [-1.0, 1.0] normal component range to [-512, 511] 10-bit range.
		int32_t x = static_cast<int32_t>(normalX * 511.0f);
		int32_t y = static_cast<int32_t>(normalY * 511.0f);
		int32_t z = static_cast<int32_t>(normalZ * 511.0f);
		int32_t w = 0; // Unused space.

		// Put the bits in their proper places according to OpenGL specification.
		return (w & 0x3) | ((z & 0x3FF) << 2) | ((y & 0x3FF) << 12) | ((x & 0x3FF) << 22);

	}

}