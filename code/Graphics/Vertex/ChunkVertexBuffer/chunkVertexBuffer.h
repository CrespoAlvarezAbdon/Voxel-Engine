#ifndef _VOXELENG_CHUNKVERTEXBUFFER_
#define _VOXELENG_CHUNKVERTEXBUFFER_

#include <unordered_map>
#include <set>
#include <utility>
#include <Graphics/Vertex/VertexBuffer/vertexBuffer.h>
#include <vec.h>

namespace VoxelEng {

	////////////
	//Structs.//
	////////////

	/**
	* @brief Represents a portion of the chunk vertex buffer.
	*/
	struct chunkVertexBufferZone {

		long long startPos; // Index of the first byte.
		long long size; // Zone size in bytes.

	};

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
		const chunkVertexBufferZone& bufferZone(const vec3& chunkPos, bool isTranslucidGeometry);


		// Modifiers.

		/**
		* @brief Push the given data next to the last pushed portion of data.
		* It will attempt to reuse a freed portion of the buffer to allocate this data
		* if available to reduce memory fragmentation.
		* @param chunkPos The position in chunk coordinates of the chunk whose data is being pushed into the VBO.
		* @param data Pointer to the begininng of the given data to push.
		* @param size The given data's size. Must be greater than 0 or otherwise an exception will be thrown.
		* @param isTranslucidGeometry Whether the given vertex data corresponds to translucid geometry (true)
		* or not (false).
		*/
		void pushDynamicData(const vec3& chunkPos, const void* data, long long size, bool isTranslucidGeometry);

		/**
		* @brief Will mark the zone of the buffer memory reserved for the given chunk as free.
		* This zone will not be rendered when calling to chunkVertexBuffer::renderAll().
		* The given chunk must have a zone of the buffer memory reserved for it, otherwise it will do nothing.
		* @param chunkPos The chunk position corresponding to the reserved buffer zone to free.
		* @param isTranslucidGeometry Whether the given vertex data corresponds to translucid geometry (true)
		* or not (false).
		*/
		void freeDynamicData(const vec3& chunkPos, bool isTranslucidGeometry);

	private:

		// Nested classes/structs.

		struct compareBySize {

			bool operator()(const chunkVertexBufferZone& a, const chunkVertexBufferZone& b) const {

				return a.size > b.size && a.startPos > b.startPos;

			}

		};
		typedef std::set<chunkVertexBufferZone, compareBySize> freedZonesBySize;

		struct compareByStartPos {

			bool operator()(const freedZonesBySize::iterator& a, const freedZonesBySize::iterator& b) const {

				return a->startPos < b->startPos;

			}

		};
		typedef std::set<freedZonesBySize::iterator, compareByStartPos> freedZones;

		std::unordered_map<vec3, chunkVertexBufferZone> chunkVertexBufferZones_;
		std::unordered_map<vec3, chunkVertexBufferZone> chunkTranslucidVertexBufferZones_;
		freedZones freedZones_;
		freedZonesBySize freedZonesBySize_;
	
	};

	inline chunkVertexBuffer::chunkVertexBuffer()
	{}

}

#endif //_VOXELENG_CHUNKVERTEXBUFFER_