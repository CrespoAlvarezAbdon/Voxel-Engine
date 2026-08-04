#ifndef _VOXELENG_CHUNK_BLOCK_DATA_
#define _VOXELENG_CHUNK_BLOCK_DATA_

#include <definitions.h>
#include <unordered_set>
#include <Utilities/Padded3DArray/Padded3DArray.hpp>
#include <vec.h>

namespace VoxelEng {

	struct chunkBlockData {

		chunkBlockData();

		chunkBlockData(const chunkBlockData& blockData);

		Padded3DArray<unsigned short> blocksLocalIDs;
		Padded3DArray<byte> isOpaque;
		Padded3DArray<blockLight>* blockLightColor; // Lighting color value in the specific block without light level applied. 4ºth value is alpha.
		std::unordered_set<ivec3> floodPointLightPositions;

	};

	inline chunkBlockData::chunkBlockData()
	: blocksLocalIDs(CHUNK_SIZE, CHUNK_SIZE, CHUNK_SIZE, 1, 0),
	  isOpaque(CHUNK_SIZE, CHUNK_SIZE, CHUNK_SIZE, 1, static_cast<byte>(0)),
	  blockLightColor(nullptr) {}

	inline chunkBlockData::chunkBlockData(const chunkBlockData& blockData)
	: blocksLocalIDs(blockData.blocksLocalIDs),
      isOpaque(blockData.isOpaque),
	  floodPointLightPositions (blockData.floodPointLightPositions)
	{
		blockLightColor = blockData.blockLightColor ? new Padded3DArray<blockLight>(*blockData.blockLightColor) : nullptr;
	}

}

#endif