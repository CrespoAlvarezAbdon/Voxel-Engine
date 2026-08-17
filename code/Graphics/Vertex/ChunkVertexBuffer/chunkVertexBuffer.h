#ifndef _VOXELENG_CHUNK_VERTEX_BUFFER_
#define _VOXELENG_CHUNK_VERTEX_BUFFER_

#include <unordered_map>
#include <set>
#include <utility>
#include <vec.h>
#include <Chunk/chunkRenderingData.hpp>
#include <Graphics/graphicsDefinitions.h>
#include <Graphics/Vertex/ChunkVertexBuffer/chunkVertexBufferZone.hpp>
#include <Graphics/Vertex/VertexBuffer/vertexBuffer.h>

namespace VoxelEng {

	/**
	* @brief Specialization of 'vertexBuffer' class that defines a buffer
	* to store chunk vertex data in GPU and additionally store information about
	* the positions of each chunk's vertex data and the size they occupy in the buffer.
	* It's purpose is to store all the chunk data at once in GPU and be able to keep track
	* of changes in those chunks (loading, modification and unloading of chunks) and properly
	* update the corresponding vertex data in GPU.
	*/
	class chunkVertexBuffer : public vertexBuffer {
	
	public:

		// Constructors.

		/**
		* @brief Default class constructor.
		*/
		chunkVertexBuffer();


		// Observers.

		/**
		* @brief Get the buffer zone for the specified chunk. Throws exception if there is no buffer zone associated
		* with the specified chunk.
		* @param chunkPos The position in chunk coordinates of the specified chunk.
		* @param isTranslucidGeometry Whether the given vertex data corresponds to translucid geometry (true)
		* or not (false).
		* @returns The buffer zone for the specified chunk.
		*/
		const chunkVertexBufferZone& bufferZone(const ivec3& chunkPos, geometryType chunkGeometryType);


		// Modifiers.

		/**
		* @brief Push the given data next to the last pushed portion of data.
		* It will attempt to reuse a freed portion of the buffer to allocate this data
		* if available to reduce memory fragmentation.
		* @param chunkPos The position in chunk coordinates of the chunk whose data is being pushed into the VBO.
		* @param chunkRenderData Chunk's rendering data to push into the VBO.
		* @param isTranslucidGeometry Whether the given vertex data corresponds to translucid geometry (true)
		* or not (false).
		*/
		void pushDynamicData(const ivec3& chunkPos, const chunkRenderingData& chunkRenderData, geometryType chunkGeometryType);

		/**
		* @brief Will mark the zone of the buffer memory reserved for the given chunk as free.
		* This zone will not be rendered when calling to chunkVertexBuffer::renderAll().
		* The given chunk must have a zone of the buffer memory reserved for it, otherwise it will do nothing.
		* @param chunkPos The chunk position corresponding to the reserved buffer zone to free.
		* @param isTranslucidGeometry Whether the given vertex data corresponds to translucid geometry (true)
		* or not (false).
		*/
		void freeDynamicData(const ivec3& chunkPos, geometryType chunkGeometryType);

	private:

		// Nested classes/structs.

		struct compareBySize {

			bool operator()(const chunkVertexBufferZone& a, const chunkVertexBufferZone& b) const {

				if (a.size != b.size)
					return a.size < b.size; // ascending by size
				else
					return a.startPos < b.startPos;   // tie-breaker

			}

		};
		typedef std::set<chunkVertexBufferZone, compareBySize> freedZonesBySize;

		struct compareByStartPos {

			bool operator()(const freedZonesBySize::iterator& a, const freedZonesBySize::iterator& b) const {

				return a->startPos < b->startPos;

			}

		};
		typedef std::set<freedZonesBySize::iterator, compareByStartPos> freedZones;

		std::unordered_map<ivec3, chunkVertexBufferZone> chunkVertexBufferZones_;
		std::unordered_map<ivec3, chunkVertexBufferZone> chunkTranslucidVertexBufferZones_;
		freedZones freedZones_;
		freedZonesBySize freedZonesBySize_;
	
	};

	inline chunkVertexBuffer::chunkVertexBuffer()
	{}

}

#endif //_VOXELENG_CHUNKVERTEXBUFFER_