#ifndef _VOXELENG_CHUNK_DEFINITIONS_
#define _VOXELENG_CHUNK_DEFINITIONS_

#include <array>

#include <definitions.h>
#include <vec.h>

namespace VoxelEng {

    /**
    * @brief Number of chunk neighbors that a certain chunk has. Neighbor chunks
    * are those that share a border with the chunk.
    */
    const unsigned int CHUNK_NEIGHBORS = 26;

    /**
    * @brief All the possible offsets for all the possible neighbors that a chunk may have.
    */
	std::array<vec3, CHUNK_NEIGHBORS> neighborsOffsets = { 

        vec3{-1, -1, -1}, vec3{-1, -1,  0}, vec3{-1, -1,  1},
        vec3{-1,  0, -1}, vec3{-1,  0,  0}, vec3{-1,  0,  1},
        vec3{-1,  1, -1}, vec3{-1,  1,  0}, vec3{-1,  1,  1},

        vec3{ 0, -1, -1}, vec3{ 0, -1,  0}, vec3{ 0, -1,  1},
        vec3{ 0,  0, -1},                   vec3{ 0,  0,  1},
        vec3{ 0,  1, -1}, vec3{ 0,  1,  0}, vec3{ 0,  1,  1},

        vec3{ 1, -1, -1}, vec3{ 1, -1,  0}, vec3{ 1, -1,  1},
        vec3{ 1,  0, -1}, vec3{ 1,  0,  0}, vec3{ 1,  0,  1},
        vec3{ 1,  1, -1}, vec3{ 1,  1,  0}, vec3{ 1,  1,  1}

	};

}

#endif