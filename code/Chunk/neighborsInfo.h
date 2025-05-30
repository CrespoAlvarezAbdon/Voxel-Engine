#ifndef _VOXELENG_NEIGHBORS_INFO_
#define _VOXELENG_NEIGHBORS_INFO_

#include <atomic>
#include <list>

#include <Chunk/blockLightMod.h>
#include <Utilities/Thread/threadsafe.hpp>

namespace VoxelEng {

	struct neighborsInfo {
	
		std::atomic<unsigned char> neighborsGenPass1Completed_;

		threadsafe<std::list<blockLightMod>> blockLightModsFromPlusX;
		threadsafe<std::list<blockLightMod>> blockLightModsFromMinusX;
		threadsafe<std::list<blockLightMod>> blockLightModsFromPlusY;
		threadsafe<std::list<blockLightMod>> blockLightModsFromMinusY;
		threadsafe<std::list<blockLightMod>> blockLightModsFromPlusZ;
		threadsafe<std::list<blockLightMod>> blockLightModsFromMinusZ;
	
	};

}

#endif