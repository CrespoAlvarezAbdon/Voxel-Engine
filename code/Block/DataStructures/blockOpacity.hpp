#ifndef _VOXELENG_BLOCK_OPACITY_
#define _VOXELENG_BLOCK_OPACITY_

#include <definitions.h>

namespace VoxelEng {

	/**
	* @brief Defines how much a block restricts the passing of light through it.
	* Since light level values range from 0 to 8, there are 8 opacity levels from TRANSPARENT to OPAQUE
	*/
	enum class blockOpacity : byte {

		NONE = 0,
		TRANSPARENT,
		OPACITY_ONE,
		OPACITY_TWO,
		OPACITY_THREE,
		OPACITY_FOUR,
		OPACITY_FIVE,
		OPACITY_SIX,
		OPAQUE

	};

	inline bool isTranslucent(blockOpacity opacity) {
	
		return opacity > blockOpacity::TRANSPARENT && opacity < blockOpacity::OPAQUE;
	
	}

}

#endif

