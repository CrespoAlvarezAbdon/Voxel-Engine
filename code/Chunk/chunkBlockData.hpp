#ifndef _VOXELENG_CHUNK_BLOCK_DATA_
#define _VOXELENG_CHUNK_BLOCK_DATA_

#include <unordered_set>
#include <Utilities/Padded3DArray/Padded3DArray.hpp>
#include <vec.h>

namespace VoxelEng {

	struct chunkBlockData {

		// TODO. FUSE THESE THREE ARRAYS OF THE SAME SIZE INTO ONE TO ONLY HAVE TO DO AT MOST 1 SEARCH?
		Padded3DArray<unsigned short>* blocksLocalIDs_;
		Padded3DArray<byte>* isOpaque_;
		Padded3DArray<blockLight>* blockLightColor_; // Lighting color value in the specific block without light level applied. 4ºth value is alpha.
		std::unordered_set<ivec3>* floodPointLightPositions_;

	};

}

#endif