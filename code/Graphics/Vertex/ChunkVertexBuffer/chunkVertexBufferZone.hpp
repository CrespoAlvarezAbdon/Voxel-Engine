#ifndef _VOXELENG_CHUNK_VERTEX_BUFFER_ZONE_
#define _VOXELENG_CHUNK_VERTEX_BUFFER_ZONE_

#include <Chunk/DataStructures/chunkRenderingData.hpp>

namespace VoxelEng {

	// MAÑANA. APLICAR CAMBIOS.
	// -EXTRARENDERINGDATA VA EN UN UNORDERED_MAP APARTE EN CHUNKSVBO. <- DONE
	// -EXTRARENDERINGDATA AHORA CONTIENE ISOPAQUE DE CHUNKBLOCKDATA. <- DONE.
	// -SUBIR ISOPAQUE A GPU.
	// -METER LISTA LUCES EN EXTRARENDERINGDATA.
	// -SUBIR LISTA LUCES A GPU.

	/**
	* @brief Represents a portion of the chunk vertex buffer.
	*/
	struct chunkVertexBufferZone {

		/*
		Methods.
		*/

		// Constructors.

		/**
		* @brief Default class constructor.
		*/
		chunkVertexBufferZone();

		/**
		* @brief Class constructor.
		* @param startPos Index of the first byte of the chunk's mesh data.
		* @param size Index of the last byte of the chunk's mesh data.
		*/
		chunkVertexBufferZone(long long startPos, long long size);


		// Attributes.

		/**
		* @brief Index of the first byte of the chunk's mesh data.
		*/
		long long startPos;

		/**
		* @brief Index of the last byte of the chunk's mesh data.
		*/
		long long size;

	};

	inline chunkVertexBufferZone::chunkVertexBufferZone()
	: startPos(0), size(0) {}

	inline chunkVertexBufferZone::chunkVertexBufferZone(long long startPos, long long size)
	: startPos(startPos), size(size) {}

}

#endif