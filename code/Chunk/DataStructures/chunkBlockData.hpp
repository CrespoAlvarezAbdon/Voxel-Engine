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
		Padded3DArray<blockLight>* blockLightColor; // Lighting color value in the specific block without light level applied. 4ºth value is alpha.
		std::unordered_set<ivec3> floodPointLightPositions;
		// NEXT. HE METIDO YA LA ESTRUCTURA EN LAS SHADERS, AHORA HAY QUE RELLENAR EL SSBO EN EL BUCLE DE RENDERIZADO CUANDO TOQUE.

	};

	inline chunkBlockData::chunkBlockData()
	: blocksLocalIDs(CHUNK_SIZE, CHUNK_SIZE, CHUNK_SIZE, 1, 0),
	  
	  blockLightColor(nullptr) {}

	inline chunkBlockData::chunkBlockData(const chunkBlockData& blockData)
	: blocksLocalIDs(blockData.blocksLocalIDs),
	  floodPointLightPositions (blockData.floodPointLightPositions)
	{
		blockLightColor = blockData.blockLightColor ? new Padded3DArray<blockLight>(*blockData.blockLightColor) : nullptr;
	}

}

#endif