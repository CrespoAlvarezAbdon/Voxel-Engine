#ifndef _CHUNK_NEIGHBORS_
#define _CHUNK_NEIGHBORS_

namespace VoxelEng {

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