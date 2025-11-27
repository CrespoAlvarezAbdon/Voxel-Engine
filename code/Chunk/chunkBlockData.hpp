#ifndef _VOXELENG_CHUNK_BLOCK_DATA_
#define _VOXELENG_CHUNK_BLOCK_DATA_

#include <Utilities/Padded3DArray/Padded3DArray.hpp>
#include <vec.h>

namespace VoxelEng {

	struct chunkBlockData {

		Padded3DArray<unsigned short>* blocksLocalIDs_;
		Padded3DArray<byte>* isOpaque_;
		Padded3DArray<basicVec4>* blockLightColor_; // Lighting color value in the specific block without light level applied. 4ºth value is alpha.
		Padded3DArray<char>* blockLightLevel_; // Lighting value in the specific block.

	};

}

#endif