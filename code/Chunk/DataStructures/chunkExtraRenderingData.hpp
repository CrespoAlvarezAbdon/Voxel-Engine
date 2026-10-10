#ifndef _VOXELENG_CHUNK_EXTRA_RENDERING_DATA_
#define _VOXELENG_CHUNK_EXTRA_RENDERING_DATA_

#include <definitions.h>
#include <vec.h>
#include <Block/DataStructures/blockOpacity.hpp>
#include <Utilities/Padded3DArray/Padded3DArray.hpp>

namespace VoxelEng {

	/**
	* @brief Part of the chunk rendering data that is not mesh itself.
	*/
	struct chunkExtraRenderingData {

		/*
		Methods.
		*/

		// Constructors.

		/**
		* @brief Class constructor.
		* @param globalChunkPos Chunk's position in global grid coordinates.
		*/
		chunkExtraRenderingData(const vec3& globalChunkPos = vec3Zero);


		/*
		Attributes.
		*/

		/**
		* @brief Chunk center's position in global coordinates.
		*/
		vec3 globalChunkPos;

		/**
		* @brief Stores the transparency properties of every block in the chunk and the ones from its border with its neighbors.
		*/
		Padded3DArray<blockOpacity> opacity_; // 18x18x18 bytes = 5832 bytes = 1458 32-bit unsigned integers.
		

	};

	inline chunkExtraRenderingData::chunkExtraRenderingData(const vec3& globalChunkPos) 
	: globalChunkPos(globalChunkPos), opacity_(CHUNK_SIZE, CHUNK_SIZE, CHUNK_SIZE, 1, blockOpacity::NONE) {}

}

#endif