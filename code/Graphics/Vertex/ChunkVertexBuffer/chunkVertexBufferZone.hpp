#ifndef _VOXELENG_CHUNK_VERTEX_BUFFER_ZONE_
#define _VOXELENG_CHUNK_VERTEX_BUFFER_ZONE_

#include <Chunk/chunkExtraRenderingData.hpp>

namespace VoxelEng {

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

		/**
		* @brief Class constructor.
		* @param startPos Index of the first byte of the chunk's mesh data.
		* @param size Index of the last byte of the chunk's mesh data.
		* @param extraRenderingData Part of the chunk rendering data that is not mesh itself.
		*/
		chunkVertexBufferZone(long long startPos, long long size, const chunkExtraRenderingData& extraRenderingData);


		// Attributes.

		/**
		* @brief Index of the first byte of the chunk's mesh data.
		*/
		long long startPos;

		/**
		* @brief Index of the last byte of the chunk's mesh data.
		*/
		long long size;

		/**
		* @brief Part of the chunk rendering data that is not mesh itself.
		*/
		chunkExtraRenderingData extraRenderingData;

	};

	inline chunkVertexBufferZone::chunkVertexBufferZone()
	: startPos(0), size(0) {}

	inline chunkVertexBufferZone::chunkVertexBufferZone(long long startPos, long long size)
	: startPos(startPos), size(size) {}

	inline chunkVertexBufferZone::chunkVertexBufferZone(long long startPos, long long size, const chunkExtraRenderingData& extraRenderingData)
	: startPos(startPos), size(size), extraRenderingData(extraRenderingData) {}

}

#endif