#ifndef _VOXELENG_NEIGHBORS_INFO_
#define _VOXELENG_NEIGHBORS_INFO_

#include <atomic>

namespace VoxelEng {

	struct neighborsInfo {
	
		std::atomic<unsigned char> neighborsGenPass1Completed_;
	
	};

}

#endif