#ifndef _VOXELENG_CHUNK_DEFINITIONS_
#define _VOXELENG_CHUNK_DEFINITIONS_

#include <array>
#include <utility>
#include <unordered_map>
#include <list>

#include <definitions.h>
#include <vec.h>
#include <Block/blockState.hpp>
#include <Chunk/blockLight.hpp>

namespace VoxelEng {

    /*
    Typedefs.
    */

    typedef std::unordered_map<vec3, std::list<std::pair<basicVec3, blockState>>> blockStatesToSet; // vec3 is chunkPos, basicVec3 is block pos un local chunk grid
    typedef std::unordered_map<vec3, std::list<std::pair<basicVec3, blockLight>>> lightDataToSet; // vec3 is chunkPos, basicVec3 is block pos un local chunk grid


    /*
    Constants.
    */

    /**
    * @brief Number of chunk neighbors that a certain chunk has. Neighbor chunks
    * are those that share a border with the chunk.
    */
    const unsigned int CHUNK_NEIGHBORS = 26;

    /**
    * @brief Number of chunk neighbors that a certain chunk has plus one. Neighbor chunks
    * are those that share a border with the chunk.
    */
    const unsigned int CHUNK_NEIGHBORS_PLUS_ONE = CHUNK_NEIGHBORS + 1;

    

}

#endif