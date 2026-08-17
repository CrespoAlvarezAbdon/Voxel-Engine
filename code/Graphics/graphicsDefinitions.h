#ifndef _VOXELENG_GRAPHICS_DEFINITIONS_
#define _VOXELENG_GRAPHICS_DEFINITIONS_

namespace VoxelEng {

    /*
    Enums
    */

    /**
    * @brief Different types of geometry handled in different graphic pipeline steps by the engine.
    */
    enum class geometryType {

        NONE = 0,
        OPAQUE = 1,
        TRANSLUCENT

    };

}

#endif