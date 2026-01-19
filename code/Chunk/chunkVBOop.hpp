#ifndef _VOXELENG_CHUNK_VBO_OP_
#define _VOXELENG_CHUNK_VBO_OP_

#include <Chunk/chunkEnums.hpp>
#include <Chunk/chunkRenderingData.hpp>

namespace VoxelEng {

	/**
	* @brief Operation in the chunk's VBO.
	*/
	struct chunkVBOop {

		/*
		Methods.
		*/

		// Constructors.

		/**
		* @brief Default class constructor.
		*/
		chunkVBOop();

		/**
		* @brief Class constructor.
		* @param op VBO operation to perform.
		*/
		chunkVBOop(VBOop op);

		/**
		* @brief Class constructor.
		* @param op VBO operation to perform.
		* @param renderingData Rendering data associated with the VBO op to perform.
		*/
		chunkVBOop(VBOop op, const chunkRenderingData& renderingData);


		/*
		Attributes.
		*/

		/**
		* @brief The operation to perform.
		*/
		VBOop op;

		/**
		* @brief Operation's associated rendering data (if any).
		*/
		chunkRenderingData renderingData;

	};

	inline chunkVBOop::chunkVBOop()
	: op(VBOop::NONE) {}

	inline chunkVBOop::chunkVBOop(VBOop op)
	: op(op) {}

	inline chunkVBOop::chunkVBOop(VBOop op, const chunkRenderingData& renderingData)
	: op(op), renderingData(renderingData) {}

}

#endif