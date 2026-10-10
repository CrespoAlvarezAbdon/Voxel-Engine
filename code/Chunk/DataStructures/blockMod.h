#ifndef _VOXELENG_BLOCK_MODIFICATION_
#define _VOXELENG_BLOCK_MODIFICATION_

#include <block.h>
#include <vec.h>

namespace VoxelEng {

	////////////
	//Structs.//
	////////////

	/**
	* @brief Definition of a modification of a block in a chunk.
	*/
	struct blocktMod {

		/**
		* @brief Position the light is being propagated to.
		*/
		basicVec3 pos;

		block* b;

	};

}

#endif