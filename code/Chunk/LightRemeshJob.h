#ifndef _VOXELENG_LIGHT_REMESH_JOB_
#define _VOXELENG_LIGHT_REMESH_JOB_

#include <deque>
#include <utility>
#include <definitions.h>
#include <vec.h>
#include <Chunk/FloodLightPositions.h>

namespace VoxelEng {

	// Forward declarations.
	class chunk;

	/**
	* @brief Definition of a chunk remesh job caused by a change
	* in the lighting of some of the chunk's neighbor blocks that got propatagated into this chunk.
	*/
	struct LightRemeshJob {

		/**
		* @brief Class constructor.
		*/
		LightRemeshJob();

		/**
		* Class destructor.
		*/
		~LightRemeshJob();

		// Only 6 neighbors per chunk, so a chunk can only receive light from other 6 neighbor chunks.
		vec3 chunkPos;
		std::deque<floodLightPropInstance>* floodLightPositions;
		blockViewDir originChunkDir;
		chunk* chunkToRemesh;

	};

}

#endif