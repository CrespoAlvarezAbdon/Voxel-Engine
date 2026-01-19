#ifndef _VOXELENG_NEIGHBORS_INFO_
#define _VOXELENG_NEIGHBORS_INFO_

#include <list>
#include <unordered_map>
#include <set>
#include <vector>

#include <vec.h>
#include <Chunk/blockLightMod.h>
#include <Utilities/Thread/threadsafe.hpp>

namespace VoxelEng {

	typedef threadsafe<std::unordered_map<basicVec3, std::list<blockLightMod>>> blockLightsBySpreadLight;
	typedef std::unordered_map<ivec3, blockLightsBySpreadLight> blockLightsByNeighbor;

	struct neighborsInfo {
	
		threadsafe<std::set<ivec3>> neighborsGenPass1Completed_;
		threadsafe<blockLightsByNeighbor> blockLightsFromNeighbor;
	
	};

}

#endif