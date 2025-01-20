#include "chunkAdditionalBlockData.h"

#include <cstring>

namespace VoxelEng {

	void chunkAdditionalBlockData::clear() {
	
		std::memset(isSolid, 0, 4096 * sizeof(unsigned int)); // 4096 = 16*16*16 and isSolid is a 16x16x16 unsigned int array.
	
	}

}