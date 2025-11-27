#ifndef _VOXELENG_BLOCKSTATE_
#define _VOXELENG_BLOCKSTATE_

#include <Block/block.h>
#include <Block/Properties/blockProperty.hpp>

namespace VoxelEng {

	struct blockState {

		/**
		* Methods.
		*/

		// Constructors.
		
		/**
		* @brief Class constructor.
		* @param b Block type corresponding to the block state.
		* @param prop Properties
		*/
		blockState(const block& b, const blockProperty& prop);


		/**
		* Attributes.
		*/

		const block& b;
		const blockProperty& prop; // TODO. MAKE THIS A UNORDERED_SET AND MAKE BLOCKPROPERTY CLASS ABLE TO BE USED IN UNORDERED_SETS

	};

	inline blockState::blockState(const block& b, const blockProperty& prop)
	: b(b), prop(prop) {}

}

#endif