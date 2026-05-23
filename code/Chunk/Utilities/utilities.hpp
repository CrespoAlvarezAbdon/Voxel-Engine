#ifndef _VOXELENG_CHUNK_UTILITIES_
#define _VOXELENG_CHUNK_UTILITIES_

#include <vec.h>
#include <Graphics/Lighting/definitions.hpp>
#include <Utilities/VarRef/varRef.h>

namespace VoxelEng {

    namespace chunkUtility {
    
        void setBlockLight(const ivec3& inChunkPos, const varRef& emittedLight,
            colorChannel channel, lightIntensity& intensity, lightValue& value);
    
    }

}

#endif