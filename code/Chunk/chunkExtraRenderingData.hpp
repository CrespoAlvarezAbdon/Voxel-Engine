#ifndef _VOXELENG_CHUNK_EXTRA_RENDERING_DATA_
#define _VOXELENG_CHUNK_EXTRA_RENDERING_DATA_

#include <vec.h>

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
		* @brief Default class constructor.
		*/
		chunkExtraRenderingData();

		/**
		* @brief Class constructor.
		* @param globalChunkPos Chunk's position in global grid coordinates.
		*/
		chunkExtraRenderingData(const vec3& globalChunkPos);


		/*
		Attributes.
		*/

		/**
		* @brief Chunk center's position in global coordinates.
		*/
		vec3 globalChunkPos;

	};

	inline chunkExtraRenderingData::chunkExtraRenderingData()
	: globalChunkPos(vec3Zero) {}

	inline chunkExtraRenderingData::chunkExtraRenderingData(const vec3& globalChunkPos) 
	: globalChunkPos(globalChunkPos) {}

}

#endif