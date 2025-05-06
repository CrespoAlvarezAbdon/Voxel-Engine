#include "LightRemeshJob.h"

#include <Chunk/chunk.h>

namespace VoxelEng {

	LightRemeshJob::LightRemeshJob()
	: floodLightPositions(nullptr),
	  originChunkDir(blockViewDir::NONE),
	  chunkToRemesh(nullptr)
	{}

	LightRemeshJob::~LightRemeshJob() {
	
		if (floodLightPositions) {
			
			delete floodLightPositions;
			floodLightPositions = nullptr;
			
		}
	
	}

}