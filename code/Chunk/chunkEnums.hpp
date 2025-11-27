/**
* @file chunkEnums.hpp
* @version 1.0
* @date 30/09/2025
* @author Abdon Crespo Alvarez
* @title Chunk enums.
* @brief Chunk-related enumerations.
*/
#ifndef _VOXELENG_CHUNK_ENUMS_
#define _VOXELENG_CHUNK_ENUMS_

namespace VoxelEng {

	/**
	* @brief The different forms a chunk may exist in the chunk manager system.
	* COMMON -> Chunk that exists loaded because it is inside a player's load radius.
	* SIMULATED -> Chunk that should be unloaded but it is forced to stay because it is owned.
	*/
	enum class chunkExistence { NOEXISTS = 0, COMMON = 1, SIMULATED = 2};

	/**
	* @brief The different stages that a chunk has during its loading process.
	*/
	enum class chunkLoadStatus { NOTLOADED = 0, BASICTERRAIN = 1, BASICTERRAINFROMDISK = 2, DECORATED = 3, MESHED = 4 };

	/**
	* @brief Definition of the multiples types of jobs related to chunk management.
	*/
	enum class chunkJobType { NONE = 0, LOAD = 1, LOAD2 = 2, ONLYREMESH = 3, UNLOADANDSAVE = 4, PRIORITYREMESH = 5, SAVEONLY = 6 };

	// LOAD2 WILL BE USED FOR LIGHTING LAYER.

	/**
	* @brief Definition of the operations allowed in the chunk vertex buffer object.
	*/
	enum class chunkVBOoperation { NONE = 0, PUSH, FREE };

}

#endif