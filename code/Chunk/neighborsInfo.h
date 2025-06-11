#ifndef _VOXELENG_NEIGHBORS_INFO_
#define _VOXELENG_NEIGHBORS_INFO_

#include <atomic>
#include <list>
#include <unordered_map>

#include <vec.h>
#include <Chunk/blockLightMod.h>
#include <Utilities/Thread/threadsafe.hpp>

namespace VoxelEng {

	typedef std::unordered_map<basicVec3, threadsafe<std::list<blockLightMod>>> blockLightsByNeighbor;

	struct neighborsInfo {
	
		std::atomic<unsigned char> neighborsGenPass1Completed_;
		threadsafe<blockLightsByNeighbor> blockLightsFromNeighbor;
	
	};

}

#endif