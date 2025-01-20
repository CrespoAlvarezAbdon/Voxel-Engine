#ifndef _VOXELENG_CHUNK_ADDITIONAL_BLOCK_DATA_
#define _VOXELENG_CHUNK_ADDITIONAL_BLOCK_DATA_

#include <Registry/registryElement.h>

namespace VoxelEng {

	/**
	* @brief Additional block data is stored in this class' objects.
	*/
	class chunkAdditionalBlockData : public registryElement {

	public:

		/*
		Methods.
		*/

		// Modifiers.

		void clear();


		/*
		Attributes.
		*/

		/**
		* @brief Whether the block is solid (aka does not let light pass through it) (1) or not (0).
		*/
		unsigned int isSolid[16][16][16];

	};

}

#endif