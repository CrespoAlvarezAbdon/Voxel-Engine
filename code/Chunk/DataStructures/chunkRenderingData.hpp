#ifndef _VOXELENG_CHUNK_RENDERING_DATA_
#define _VOXELENG_CHUNK_RENDERING_DATA_

#include <definitions.h>
#include <Chunk/DataStructures/chunkExtraRenderingData.hpp>
#include <Graphics/Models/model.h>

namespace VoxelEng {

	/**
	* @brief Wraps up data used to render a chunk.
	*/
	struct chunkRenderingData {

		/**
		* Methods.
		*/

		// Constructors.

		/**
		* @brief Default class constructor.
		*/
		chunkRenderingData();


		/*
		Attributes.
		*/

		/**
		* @brief Chunk's opaque geometry vertices.
		*/
		model vertices;

		/**
		* @brief Chunk's translucent geometry vertices.
		*/
		model translucentVertices;

		/**
		* @brief Total size of the chunk's vertices.
		*/
		unsigned int totalSize;

		chunkExtraRenderingData extraData;

	};

	inline chunkRenderingData::chunkRenderingData() 
	: totalSize(0)
	{}

}

#endif