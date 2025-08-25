/**
* @file chunk.h
* @version 1.0
* @date 20/04/2023
* @author Abdon Crespo Alvarez
* @title Chunk.
* @brief Contains the chunk and chunkManager classes as well as some
* auxiliary data structures and types used with them.
*/
#ifndef _VOXELENG_CHUNK_
#define _VOXELENG_CHUNK_

#include <atomic>
#include <barrier>
#include <condition_variable>
#include <cstddef>
#include <deque>
#include <list>
#include <thread>
#include <mutex>
#include <set>
#include <shared_mutex>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>
#include <time.h>
#include <functional>
#include <atomicRecyclingPool.h>
#include <block.h>
#include <definitions.h>
#include <event.h>
#include <listener.h>
#include <threadPool.h>
#include <palette.h>
#include <vec.h>
#include <utilities.h>
#include <Chunk/blockLightMod.h>
#include <Chunk/neighborsInfo.h>
#include <Graphics/Lighting/Lights/LightInstance/lightInstance.h>
#include <Graphics/Textures/texture.h>
#include <Graphics/Shaders/shader.h>
#include <Graphics/Models/model.h>
#include <Graphics/Vertex/vertex.h>
#include <Graphics/Vertex/VertexBufferLayout/vertexBufferLayout.h>
#include <Registry/RegistryInsOrdered/registryInsOrdered.h>
#include <Utilities/BlockViewDir/blockViewDir.hpp>
#include <Utilities/Padded3DArray/Padded3DArray.hpp>

#if GRAPHICS_API == OPENGL

#include <GL/glew.h>
#include <glm.hpp>
#include <hash.hpp>

#endif

namespace VoxelEng {

	/////////////////////////
	//Forward declarations.//
	/////////////////////////

	class chunkManager;
	class chunkVertexBuffer;


	/////////////////
	//Enum classes.//
	/////////////////

	/**
	* @brief The different stages that a chunk has during its lifetime.
	*/
	enum class chunkStatus { NOTLOADED = 0, BASICTERRAIN = 1, BASICTERRAINFROMDISK = 2, DECORATED = 3, MESHED = 4};

	/**
	* @brief Definition of the multiples types of jobs related to chunk management.
	*/
	enum class chunkJobType { NONE = 0, LOAD = 1, LOAD2 = 2, ONLYREMESH = 3, UNLOADANDSAVE = 4, PRIORITYREMESH = 5};

	// LOAD2 WILL BE USED FOR LIGHTING LAYER.

	/**
	* @brief Definition of the operations allowed in the chunk vertex buffer object.
	*/
	enum class chunkVBOoperation { NONE = 0, PUSH, FREE };


	////////////
	//Structs.//
	////////////

	/**
	* @brief Wraps up data used to render a chunk.
	*/
	struct chunkRenderingData {

		vec3 globalChunkPos = vec3Zero;

		model vertices;
		model translucentVertices;

		std::vector<lightInstance> pointLights;
		std::vector<lightInstance> spotLights;

		unsigned int totalSize = 0;

	};


	////////////
	//Classes.//
	////////////


	/**
	* @brief Represents a section of the voxel world, with its blocks, mesh, position and other infomation.
	* Chunks that are marked as 'dirty' or 'changed' will have their mesh regenerated.
	*/
	class chunk {

	public:

		// Initialisers.

		/**
		* @brief Initialise static members of the chunk class.
		* Allocate any resources that are needed on initialisation.
		*/
		static void init();


		// Constructors.

		/**
		* @brief Default constructor. Sets all attributes to default values
		* and does not generate anything in the chunk.
		*/
		chunk();

		/**
		* @brief Construct an empty dirty chunk.
		* If 'empty' is true then the chunk block data will be filled with null block IDs (block ID 0).
		* If 'empty' is false then the chunk block data will be filled with the selected world generator.
		*/
		chunk(bool empty, const vec3& chunkPos = vec3Zero);

		/**
		* @brief Copy constructor.
		*/
		chunk(chunk& source);


		// Observers.

		/**
		* @brief Returns true if the static members of this class are initialised
		* or false otherwise.
		*/
		static bool initialised();

		/**
		* @brief Returns the pointer to the first element of the chunk's block array.
		*/
		const Padded3DArray<unsigned short>& blocks() const;

		/**
		* @brief Get the block at the specified chunk-local coordinates.
		*/
		const block& getBlock(GLbyte x, GLbyte y, GLbyte z, bool lock);

		/**
		* @brief Get the block at the specified chunk-local coordinates.
		*/
		const block& getBlock(const vec3& inChunkPos, bool lock);

		/**
		* @brief Get the block at the specified chunk-local coordinates.
		* @param firstIndex First dimension coordinate.
		* @param secondIndex Second dimension coordinate.
		* @param neighbor Direction to follow to reach the neighbor chunk.
		*/
		const block& getNeighborBlock(GLbyte firstIndex, GLbyte secondIndex, blockViewDir neighbor);

		/**
		* @brief Get chunk's x axis coordinate (chunk-grid coordinate system).
		*/
		GLbyte x() const;

		/**
		* @brief Get chunk's y axis coordinate (chunk-grid coordinate system).
		*/
		GLbyte y() const;

		/**
		* @brief Get chunk's z axis coordinate (chunk-grid coordinate system).
		*/
		GLbyte z() const;

		/**
		* @brief Get chunk's coordinate (chunk-grid coordinate systems).
		*/
		const vec3& chunkPos() const;

		/**
		* @brief Get this chunk's global position.
		*/
		const vec3& pos() const;

		/**
		* @brief Get this chunk's rendering data object.
		*/
		const chunkRenderingData& renderingData() const;

		/**
		* @brief Returns true if this chunk has been modified
		* since being generated or loaded from disk or
		* false otherwise.
		*/
		bool modified() const;

		/**
		* @brief Get whether this chunk's mesh needs to be regenerated or not.
		* @return Whether this chunk's mesh needs to be regenerated (true) or not (false).
		*/
		bool needsRemesh() const;

		/**
		* @brief Get whether this chunk was loaded from disk or not.
		* @return Whether this chunk was loaded from disk (true) or not (false).
		*/
		bool loadedFromDisk() const;

		/**
		* @brief Returns the chunk's status.
		*/
		chunkStatus status() const;

		/**
		* @brief Get the number of non-null opaque blocks (blocks with ID != 0) that exist in the chunk.
		*/
		unsigned short nOpaqueBlocks() const;

		/**
		* @brief Returns the number of non-null opaque blocks in the chunk's +X edge.
		*/
		unsigned short nOpaqueBlocksPlusX() const;

		/**
		* @brief Returns the number of non-null opaque blocks in the chunk's -X edge.
		*/
		unsigned short nOpaqueBlocksMinusX() const;

		/**
		* @brief Returns the number of non-null opaque blocks in the chunk's +Y edge.
		*/
		unsigned short nOpaqueBlocksPlusY() const;

		/**
		* @brief Returns the number of non-null opaque blocks in the chunk's -Y edge.
		*/
		unsigned short nOpaqueBlocksMinusY() const;

		/**
		* @brief Returns the number of non-null opaque blocks in the chunk's +Z edge.
		*/
		unsigned short nOpaqueBlocksPlusZ() const;

		/**
		* @brief Returns the number of non-null opaque blocks in the chunk's -Z edge.
		*/
		unsigned short nOpaqueBlocksMinusZ() const;

		/**
		* @brief Get the total number of non-null blocks (blocks with ID != 0) that exist in the chunk.
		*/
		unsigned short nTotalBlocks() const;

		/**
		* @brief Returns the total number of non-null blocks in the chunk's +X edge.
		*/
		unsigned short nTotalBlocksPlusX() const;

		/**
		* @brief Returns the total number of non-null blocks in the chunk's -X edge.
		*/
		unsigned short nTotalBlocksMinusX() const;

		/**
		* @brief Returns the total number of non-null blocks in the chunk's +Y edge.
		*/
		unsigned short nTotalBlocksPlusY() const;

		/**
		* @brief Returns the total number of non-null blocks in the chunk's -Y edge.
		*/
		unsigned short nTotalBlocksMinusY() const;

		/**
		* @brief Returns the total number of non-null blocks in the chunk's +Z edge.
		*/
		unsigned short nTotalBlocksPlusZ() const;

		/**
		* @brief Returns the total number of non-null blocks in the chunk's -Z edge.
		*/
		unsigned short nTotalBlocksMinusZ() const;

		/**
		* @brief Returns true if the specified block in in-chunk coordinates is
		* the empty block or false otherwise.
		*/
		bool isEmptyBlock(GLbyte x, GLbyte y, GLbyte z) const;

		/**
		* @brief Returns true if the specified block in in-chunk coordinates is
		* the empty block or false otherwise.
		*/
		bool isEmptyBlock(const vec3& inChunkPos) const;

		/**
		* @brief Returns true if the specified block in in-chunk coordinates is
		* the empty block or false otherwise.
		* @param firstIndex. First dimension coordinate of the block.
		* @param secondIndex. Second dimension coordinate of the block.
		* @param neighbor. The direction to follow to reach the neighbor.
		*/
		bool isEmptyNeighborBlock(unsigned int firstIndex, unsigned int secondIndex, blockViewDir neighbor);

		/**
		* @brief Returns the chunk's palette that maps the local block IDs with the global block IDs.
		*/
		const palette<unsigned short, unsigned int>& getPalette() const;

		/**
		* @brief Returns the chunk's palette count that contains the number of global IDs mapped to
		* local IDs for this chunk.
		*/
		const std::unordered_map<unsigned short, unsigned short>& getPaletteCount() const;

		/**
		* @brief Get the free local IDs available for this chunk's block palette.
		* @returns The free local IDs available for this chunk's block palette.
		*/
		const std::unordered_set<unsigned short>& getFreeLocalIDs() const;

		/**
		* @brief Get the positions of all the point lights affecting this chunk.
		* @returns The positions of all the point lights affecting this chunk.
		*/
		const std::unordered_set<vec3>& getFloodPointLightPositions() const;




		// Modifiers.

		/**
		* @brief Returns the pointer to the first element of the chunk's block array.
		*/
		Padded3DArray<unsigned short>& blocks();

		/**
		* @brief Returns the chunk's palette that maps the local block IDs with the global block IDs.
		*/
		palette<unsigned short, unsigned int>& getPalette();

		/**
		* @brief Returns the chunk's palette count that contains the number of global IDs mapped to
		* local IDs for this chunk.
		*/
		std::unordered_map<unsigned short, unsigned short>& getPaletteCount();

		/**
		* @brief Get the free local IDs available for this chunk's block palette.
		* @returns The free local IDs available for this chunk's block palette.
		*/
		std::unordered_set<unsigned short>& getFreeLocalIDs();

		/**
		* @brief Get the positions of all the point lights affecting this chunk.
		* @returns The positions of all the point lights affecting this chunk.
		*/
		std::unordered_set<vec3>& getFloodPointLightPositions();

		/**
		* @brief Sets the value of a block within the chunk.
		* Returns the replaced block.
		* 'modification' tells if the call to this method is NOT part of the
		* chunk's generation process or otherwise.
		* WARNING. For world generators that use this method: 'modification' must be set to false.
		*/
		const block& setBlock(GLbyte x, GLbyte y, GLbyte z, const block& block, bool modification = true);

		/**
		* @brief Sets the value of a block within the chunk.
		* Returns the replaced block.
		* 'modification' tells if the call to this method is NOT part of the
		* chunk's generation process or otherwise.
		* WARNING. For world generators that use this method: 'modification' must be set to false.
		*/
		const block& setBlock(const vec3& chunkRelPos, const block& block, bool modification = true);

		/**
		* @brief Sets the value of a block within the chunk.
		* Returns the replaced block.
		* 'modification' tells if the call to this method is NOT part of the
		* chunk's generation process or otherwise. For world generators that use this method, it must be set to false.
		*/
		const block& setBlock(unsigned int linearIndex, const block& block, bool modification = true);

		/**
		* @brief Set a neighbor block.
		* A neighbor block is a copy of a block that is bordering this chunk. This copy is stored for mesh optimization purposes.
		* 'modification' tells if the call to this method is NOT part of the
		* chunk's generation process or otherwise. For world generators that use this method, it must be set to false.
		*/
		void setBlockNeighbor(unsigned int firstIndex, unsigned int secondIndex, blockViewDir neighbor, const block& block, bool modification = true);

		/**
		*
		*/
		void setBlockLight(const block& oldB, const block& b, const vec3& pos);

		/**
		* @brief Set the chunk's chunk position.
		*/
		void chunkPos(const vec3& newChunkPos);

		/**
		* @brief Returns the chunk's rendering data object.
		*/
		chunkRenderingData& renderingData();

		/**
		* @brief Locks the block data mutex.
		*/
		std::shared_mutex& blockDataMutex();

		/**
		* @brief Locks the rendering data mutex for shared ownership.
		*/
		void lockSharedRenderingDataMutex();

		/**
		* @brief Returns true if the rendering data mutex could be locked or false otherwise.
		*/
		bool tryLockRenderingDataMutex();

		/**
		* @brief Unlocks the rendering data mutex for shared ownership.
		* WARNING. Said lock must be unlock by the same thread that locked it previously.
		*/
		void unlockSharedRenderingDataMutex();

		/**
		* @brief Unlocks the rendering data mutex.
		* WARNING. Said lock must be unlock by the same thread that locked it previously.
		*/
		void unlockRenderingDataMutex();

		/**
		* @brief Set wheter this chunk's terrain has been modified and it's mesh needs to be regenerated (true) or not (false).
		* @param newValue Value to set to this flag.
		*/
		void needsRemesh(bool newValue);
		
		/**
		* @brief Set whether this chunk has been loaded from disk (true) or has been generated (false).
		* @param newValue Value to set to this flag.
		*/
		void loadedFromDisk(bool newValue);

		/**
		* @brief Returns true if this chunk has been modified
		* since being generated or loaded from disk or
		* false otherwise.
		*/
		void modified(bool newValue);

		/**
		* @brief Regenerate the chunk's mesh. Returns true if the mesh contains vertices or false if it is empty.
		*/
		bool renewMesh();

		/**
		* @brief Clear all data related to block light in the chunk.
		*/
		void clearBlockLight();

		/**
		* @brief Recalculate all the block lighting applied to the chunk.
		*/
		void recalculateBlockLight();

		/**
		* @brief Recalculate all the block lighting applied to the chunk by its neighbors' block lights.
		*/
		void recalculateNeighborBlockLight();

		/**
		* @brief The chunk's block data will be filled with null blocks, leaving the chunk "empty of blocks".
		*/
		void makeEmpty();

		/**
		* @brief Set the chunk's status.
		*/
		void status(chunkStatus level);

		/**
		* @brief Set the number of non-null opaque blocks (blocks with ID != 0) that exist in the chunk.
		*/
		void nOpaqueBlocks(unsigned short newValue);

		/**
		* @brief Set the number of non-null opaque blocks in the chunk's +X edge.
		*/
		void nOpaqueBlocksPlusX(unsigned short newValue);

		/**
		* @brief Set the number of non-null opaque blocks in the chunk's -X edge.
		*/
		void nOpaqueBlocksMinusX(unsigned short newValue);

		/**
		* @brief Set the number of non-null opaque blocks in the chunk's +Y edge.
		*/
		void nOpaqueBlocksPlusY(unsigned short newValue);

		/**
		* @brief Set the number of non-null opaque blocks in the chunk's -Y edge.
		*/
		void nOpaqueBlocksMinusY(unsigned short newValue);

		/**
		* @brief Set the number of non-null opaque blocks in the chunk's +Z edge.
		*/
		void nOpaqueBlocksPlusZ(unsigned short newValue);

		/**
		* @brief Set the number of non-null opaque blocks in the chunk's -Z edge.
		*/
		void nOpaqueBlocksMinusZ(unsigned short newValue);

		/**
		* @brief Set the total number of non-null blocks (blocks with ID != 0) that exist in the chunk.
		*/
		void nTotalBlocks(unsigned short newValue);

		/**
		* @brief Set the total number of non-null blocks in the chunk's +X edge.
		*/
		void nTotalBlocksPlusX(unsigned short newValue);

		/**
		* @brief Set the total number of non-null blocks in the chunk's -X edge.
		*/
		void nTotalBlocksMinusX(unsigned short newValue);

		/**
		* @brief Set the total number of non-null blocks in the chunk's +Y edge.
		*/
		void nTotalBlocksPlusY(unsigned short newValue);

		/**
		* @brief Set the total number of non-null blocks in the chunk's -Y edge.
		*/
		void nTotalBlocksMinusY(unsigned short newValue);

		/**
		* @brief Set the total number of non-null blocks in the chunk's +Z edge.
		*/
		void nTotalBlocksPlusZ(unsigned short newValue);

		/**
		* @brief Set the total number of non-null blocks in the chunk's -Z edge.
		*/
		void nTotalBlocksMinusZ(unsigned short newValue);

		/**
		* @brief Executed when the frontier chunk is unloaded.
		* Checks if the neighbors of this chunk become frontier chunks
		* after this one stops being loaded.
		*/
		void onUnloadAsFrontier();

		/**
		* @brief Things to execute just after finishing generation/loading from disk go here.
		*/
		void postGenPass();


		// Destructors.

		~chunk();


		// Clean up.
		
		/**
		* @brief Clean up any resources allocated for this system.
		*/ 
		static void reset();
		
	private:

		/*
		Attributes.
		*/

		static bool initialised_;
		static const model* blockVertices_;
		static const modelTriangles* blockTriangles_;
		static const modelNormals* blockNormals_;
		
		// Serializable data.
		palette<unsigned short, unsigned int> palette_;
		std::unordered_map<unsigned short, unsigned short> paletteCount_;
		std::unordered_set<unsigned short> freeLocalIDs_;
		std::unordered_set<vec3> floodPointLightPositions_;

		Padded3DArray<unsigned short> blocksLocalIDs_;
		Padded3DArray<bool> isOpaque_;
		Padded3DArray<basicVec4> blockLightColor_; // Lighting color value in the specific block without light level applied. 4ºth value is alpha.
		Padded3DArray<char> blockLightLevel_; // Lighting value in the specific block.
		
		bool modified_;
		
		std::atomic<short> nOpaqueBlocks_;
		std::atomic<short> nOpaqueBlocksPlusX_;
		std::atomic<short> nOpaqueBlocksMinusX_;
		std::atomic<short> nOpaqueBlocksPlusY_;
		std::atomic<short> nOpaqueBlocksMinusY_;
		std::atomic<short> nOpaqueBlocksPlusZ_;
		std::atomic<short> nOpaqueBlocksMinusZ_;
		std::atomic<short> nTotalBlocks_;
		std::atomic<short> nTotalBlocksPlusX_;
		std::atomic<short> nTotalBlocksMinusX_;
		std::atomic<short> nTotalBlocksPlusY_;
		std::atomic<short> nTotalBlocksMinusY_;
		std::atomic<short> nTotalBlocksPlusZ_;
		std::atomic<short> nTotalBlocksMinusZ_;
		std::atomic<bool> needsRemesh_;
		std::atomic<bool> loadedFromDisk_;

		std::atomic<chunkStatus> loadLevel_;

		vec3 chunkPos_;

		chunkRenderingData renderingData_;
		std::shared_mutex renderingDataMutex_;

		/*
		Used for reading the block data in a chunk.
		All meshing threads only read this data, so they access it
		in shared mode, while any thread that can alter this data
		must access it in exclusive/unique mode.
		*/
		std::shared_mutex blocksMutex_;


		/*
		Methods.
		*/
		
		void placeNewBlock(unsigned short& oldLocalID, const block& newBlock);

		basicVec4 getBlockLightAverage(const basicVec4& blockLightOwn,
			const basicVec3& blockLightCoords1, const basicVec3& blockLightCoords2, const basicVec3& blockLightCords3);

	};

	inline const Padded3DArray<unsigned short>& chunk::blocks() const {

		return blocksLocalIDs_;

	}

	inline bool chunk::initialised() {
	
		return initialised_;
	
	}

	inline const block& chunk::getBlock(const vec3& inChunkPos, bool lock) {

		return getBlock(inChunkPos.x, inChunkPos.y, inChunkPos.z, lock);

	}

	inline GLbyte chunk::x() const {

		return chunkPos_.x;

	}

	inline GLbyte chunk::y() const {

		return chunkPos_.y;

	}

	inline GLbyte chunk::z() const {

		return chunkPos_.z;

	}

	inline const vec3& chunk::chunkPos() const {

		return chunkPos_;

	}

	inline const vec3& chunk::pos() const {

		return vec3{ (float)chunkPos_.x * CHUNK_SIZE, (float)chunkPos_.y * CHUNK_SIZE, (float)chunkPos_.z * CHUNK_SIZE };

	}

	inline const chunkRenderingData& chunk::renderingData() const {

		return renderingData_;

	}

	inline bool chunk::modified() const {

		return modified_;

	}

	inline bool chunk::needsRemesh() const {

		return needsRemesh_;

	}

	inline bool  chunk::loadedFromDisk() const {
	
		return loadedFromDisk_;
	
	}

	inline chunkStatus chunk::status() const {

		return loadLevel_;

	}

	inline unsigned short chunk::nOpaqueBlocks() const {

		return nOpaqueBlocks_;

	}

	inline unsigned short chunk::nOpaqueBlocksPlusX() const {
	
		return nOpaqueBlocksPlusX_;
	
	}

	inline unsigned short chunk::nOpaqueBlocksMinusX() const {

		return nOpaqueBlocksMinusX_;

	}

	inline unsigned short chunk::nOpaqueBlocksPlusY() const {

		return nOpaqueBlocksPlusY_;

	}

	inline unsigned short chunk::nOpaqueBlocksMinusY() const {

		return nOpaqueBlocksMinusY_;

	}

	inline unsigned short chunk::nOpaqueBlocksPlusZ() const {

		return nOpaqueBlocksPlusZ_;

	}

	inline unsigned short chunk::nOpaqueBlocksMinusZ() const {

		return nOpaqueBlocksMinusZ_;

	}

	inline unsigned short chunk::nTotalBlocks() const {

		return nTotalBlocks_;

	}

	inline unsigned short chunk::nTotalBlocksPlusX() const {

		return nTotalBlocksPlusX_;

	}

	inline unsigned short chunk::nTotalBlocksMinusX() const {

		return nTotalBlocksMinusX_;

	}

	inline unsigned short chunk::nTotalBlocksPlusY() const {

		return nTotalBlocksPlusY_;

	}

	inline unsigned short chunk::nTotalBlocksMinusY() const {

		return nTotalBlocksMinusY_;

	}

	inline unsigned short chunk::nTotalBlocksPlusZ() const {

		return nTotalBlocksPlusZ_;

	}

	inline unsigned short chunk::nTotalBlocksMinusZ() const {

		return nTotalBlocksMinusZ_;

	}

	inline bool chunk::isEmptyBlock(GLbyte x, GLbyte y, GLbyte z) const {
	
		return blocksLocalIDs_.at(x,y,z) == 0;
	
	}

	inline bool chunk::isEmptyBlock(const vec3& inChunkPos) const {
	
		return isEmptyBlock(inChunkPos.x, inChunkPos.y, inChunkPos.z);
	
	}

	inline const palette<unsigned short, unsigned int>& chunk::getPalette() const {
	
		return palette_;
	
	}

	inline const std::unordered_map<unsigned short, unsigned short>& chunk::getPaletteCount() const {
	
		return paletteCount_;
	
	}

	inline const std::unordered_set<unsigned short>& chunk::getFreeLocalIDs() const {
	
		return freeLocalIDs_;
	
	}

	inline const std::unordered_set<vec3>& chunk::getFloodPointLightPositions() const {
	
		return floodPointLightPositions_;
	
	}

	inline Padded3DArray<unsigned short>& chunk::blocks() {
	
		return blocksLocalIDs_;
	
	}

	inline palette<unsigned short, unsigned int>& chunk::getPalette() {
	
		return palette_;
	
	}

	inline std::unordered_map<unsigned short, unsigned short>& chunk::getPaletteCount() {
	
		return paletteCount_;
	
	}

	inline std::unordered_set<unsigned short>& chunk::getFreeLocalIDs() {
	
		return freeLocalIDs_;
	
	}

	inline std::unordered_set<vec3>& chunk::getFloodPointLightPositions() {

		return floodPointLightPositions_;

	}

	inline const block& chunk::setBlock(const vec3& chunkRelPos, const block& b, bool modification) {

		return setBlock(chunkRelPos.x, chunkRelPos.y, chunkRelPos.z, b, modification);

	}

	inline const block& chunk::setBlock(unsigned int linearIndex, const block& b, bool modification) {

		return setBlock(linearToVec3(linearIndex, CHUNK_SIZE, CHUNK_SIZE), b, modification);

	}

	inline chunkRenderingData& chunk::renderingData() {

		return renderingData_;

	}

	inline std::shared_mutex& chunk::blockDataMutex() {
	
		return blocksMutex_;
	
	}

	inline void chunk::lockSharedRenderingDataMutex() {
	
		renderingDataMutex_.lock_shared();
	
	}

	inline bool chunk::tryLockRenderingDataMutex() {

		 return renderingDataMutex_.try_lock();

	}

	inline void chunk::unlockSharedRenderingDataMutex() {
	
		renderingDataMutex_.unlock_shared();
	
	}

	inline void chunk::unlockRenderingDataMutex() {

		renderingDataMutex_.unlock();

	}

	inline void chunk::needsRemesh(bool newValue) {

		needsRemesh_ = newValue;

	}

	inline void chunk::loadedFromDisk(bool newValue) {
	
		loadedFromDisk_ = newValue;
	
	}

	inline void chunk::modified(bool newValue) {

		modified_ = newValue;

	}

	inline void chunk::status(chunkStatus level) {

		loadLevel_ = level;

	}

	inline void chunk::nOpaqueBlocks(unsigned short newValue) {

		nOpaqueBlocks_ = newValue;

	}

	inline void chunk::nOpaqueBlocksPlusX(unsigned short newValue) {

		nOpaqueBlocksPlusX_ = newValue;

	}

	inline void chunk::nOpaqueBlocksMinusX(unsigned short newValue) {

		nOpaqueBlocksMinusX_ = newValue;

	}

	inline void chunk::nOpaqueBlocksPlusY(unsigned short newValue) {

		nOpaqueBlocksPlusY_ = newValue;

	}

	inline void chunk::nOpaqueBlocksMinusY(unsigned short newValue) {

		nOpaqueBlocksMinusY_ = newValue;

	}

	inline void chunk::nOpaqueBlocksPlusZ(unsigned short newValue) {

		nOpaqueBlocksPlusZ_ = newValue;

	}

	inline void chunk::nOpaqueBlocksMinusZ(unsigned short newValue) {

		nOpaqueBlocksMinusZ_ = newValue;

	}

	inline void chunk::nTotalBlocks(unsigned short newValue) {

		nTotalBlocks_ = newValue;

	}

	inline void chunk::nTotalBlocksPlusX(unsigned short newValue) {

		nTotalBlocksPlusX_ = newValue;

	}

	inline void chunk::nTotalBlocksMinusX(unsigned short newValue) {

		nTotalBlocksMinusX_ = newValue;

	}

	inline void chunk::nTotalBlocksPlusY(unsigned short newValue) {

		nTotalBlocksPlusY_ = newValue;

	}

	inline void chunk::nTotalBlocksMinusY(unsigned short newValue) {

		nTotalBlocksMinusY_ = newValue;

	}

	inline void chunk::nTotalBlocksPlusZ(unsigned short newValue) {

		nTotalBlocksPlusZ_ = newValue;

	}

	inline void chunk::nTotalBlocksMinusZ(unsigned short newValue) {

		nTotalBlocksMinusZ_ = newValue;

	}

	//inline short chunk::setInternalBlockID(unsigned int linearIndex, short newID) {

		//return setInternalBlockID(linearIndex / (SCZ * SCY), (linearIndex / SCZ) % SCY, linearIndex % SCZ, newID);

	//}

	


	// 'chunkEvent' class.

	/**
	* @brief Event concerning a specific chunk.
	*/
	class chunkEvent : public event {

	public:

		// Constructors.

		/**
		* Class constructor.
		*/
		chunkEvent(const std::string& name);


		// Observers.

		/**
		* Returns the last chunk XZ coordinates stored within the event.
		*/
		const vec2& chunkPosXZ() const;


		// Modifiers.

		/**
		* Assign parameters that will be used by the listeners notify() method.
		*/
		void notify(int chunkPosX, int chunkPosZ);

		/**
		* Assign parameters that will be used by the listeners notify() method.
		*/
		void notify(const vec2& chunkPosXZ);

	protected:

		vec2 chunkPosXZ_;

	private:

		virtual void notify() {};

	};

	inline chunkEvent::chunkEvent(const std::string& name)
	: event(name),
		chunkPosXZ_{ 0, 0 }
	{}

	inline const vec2& chunkEvent::chunkPosXZ() const {
	
		return chunkPosXZ_;
	
	}

	inline void chunkEvent::notify(int chunkPosX, int chunkPosZ) {
	
		notify(vec2{ chunkPosX, chunkPosZ });
	
	}

	
	// 'chunkManager' class.
	
	/**
	* @brief Used for managing the chunks' life cycle, level loading...
	*/
	class chunkManager {

	public:

		/*
		Methods.
		*/

		// Initializers.

		/**
		* @brief Initialise the chunk management system.
		*/
		static void init();


		// Observers.

		/**
		* @brief Returns true if the system is initialised or false otherwise.
		*/
		static bool initialised();

		/**
		* @brief Returns the system's dictionary of registered chunks.
		*/
		static const std::unordered_map<vec3, chunk*>& chunks();

		/**
		* @brief Update the read-only copy of the chunks' meshes that are ready to be sent to GPU.
		* Only the chunks that were modified with priority updates are updated.
		*/
		static void updatePriorityReadChunkMeshes();

		/**
		* @brief Update the read-only copy of the chunks' meshes that are ready to be sent to GPU.
		* Only the chunks that were modified without priority updates are updated.
		*/
		static void updateReadChunkMeshes(std::unique_lock<std::mutex>& priorityUpdatesLock);

		/**
		* @brief Swap the write and read chunk mesh buffers so that the rendering thread may obtain the most updated one.
		*/
		static void swapChunkMeshesBuffers();

		/**
		* @brief Returns the read-only copy of chunks' meshes.
		*/
		static std::unordered_map<vec3, chunkRenderingData> const * drawableChunksRead();

		/**
		* @brief Get the read-only copy of the operations that need to be performed on
		* the rendering thread's chunk VBO.
		* @return The read-only copy of the operations that need to be performed on
		* the rendering thread's chunk VBO.
		*/
		static std::unordered_map<vec3, chunkVBOoperation> const * chunkVBOoperationsRead();

		/**
		* @brief Returns the current number of chunks to compute i n the X and Z axes from the position of the player.
		*/
		static unsigned int nChunksToCompute();

		/**
		* @brief Get the block ID of a specified block position.
		*/
		static const block& getBlock(int posX, int posY, int posZ);

		/**
		* @brief Get the block ID of a specified block position.
		*/
		static const block& getBlock(const vec3& blockPos);

		/**
		* @brief Get all blocks in the world that are in the box defined with the positions pos1 and pos2
		*/
		static std::vector<const block*> getBlocksBox(const vec3& pos1, const vec3& pos2);

		/**
		* @brief Get all blocks in the world that are in the box defined with the positions pos1 and pos2
		*/
		static std::vector<const block*> getBlocksBox(int x1, int y1, int z1, int x2, int y2, int z2);

		/**
		* @brief Returns true if the given block position is currently inside the loaded area around
		* the player or false otherwise.
		*/
		static bool isInWorld(const vec3& blockPos);

		/**
		* @brief Returns true if the given block position is currently inside the loaded area around
		* the player or false otherwise.
		*/
		static bool isInWorld(int x, int y, int z);

		/**
		* @brief Returns true if the given chunk position is inside the level's boundaries or false otherwise.
		*/
		static bool isChunkInWorld(const vec3& chunkPos);

		/**
		* @brief Returns true if the given chunk position is inside the level's boundaries or false otherwise.
		*/
		static bool isChunkInWorld(int chunkX, int chunkY, int chunkZ);

		/**
		* @brief Returns the chunk's load level.
		*/
		static chunkStatus getChunkLoadLevel(const vec3& chunkPos);

		/**
		* @brief Returns the chunk's load level.
		*/
		static chunkStatus getChunkLoadLevel(int chunkX, int chunkY, int chunkZ);

		/**
		* @brief Returns the currently opened terrain file.
		*/
		static const std::string& openedTerrainFileName();

		/**
		* @brief Returns true if the block at the specified global coordinates is an empty block
		* or false otherwise.
		*/
		static bool isEmptyBlock(int posX, int posY, int posZ);

		/**
		* @brief Returns true if the specified chunk is inside the player's render distance or
		* false otherwise.
		*/
		static bool chunkInRenderDistance(const chunk* chunk);

		/**
		* @brief Returns true if the specified chunk position is inside the player's render distance or
		* false otherwise.
		*/
		static bool chunkInRenderDistance(const vec3& chunkPos);

		/**
		* @brief Returns true if the specified chunk position is inside the player's render distance or
		* false otherwise.
		*/
		static bool chunkInRenderDistance(int chunkPosX, int chunkPosY, int chunkPosZ);

		/**
		* @brief Returns distance between the player and the specified chunk position in the three axes in chunk coordinates.
		*/
		static vec3 chunkDistanceToPlayer(const vec3& chunkPos);

		/**
		* @brief Returns the distance between two chunk positions in chunk coordinates.
		*/
		static vec3 chunkDistance(const vec3& chunkPos1, const vec3& chunkPos2);

		/**
		* @brief Returns the distance between two chunk positions in chunk coordinates.
		*/
		static vec3 chunkSignedDistance(const vec3& chunkPos1, const vec3& chunkPos2);

		/**
		* @brief Returns the SQUARED distance in global position between the specified chunk position and the player's position.
		*/
		static double distanceToPlayer(const vec3& chunkPos);

		/**
		* @brief Returns the onChunkLoad chunkEvent associated with the chunk management system.
		*/
		static const chunkEvent& onChunkLoadC();

		/**
		* @brief Returns the onChunkUnload chunkEvent associated with the chunk management system.
		*/
		static const chunkEvent& onChunkUnloadC();

		/**
		* @brief Returns the maximun number of chunks to compute according
		* to the current chunk render distance.
		*/
		static unsigned int nMaxChunksToCompute();

		/**
		* @brief Returns the maximun number of vertices to compute for a chunk.
		*/
		static unsigned int nMaxChunkVertsToCompute();

		/**
		* @brief Returns the condition variable associated with the chunk priority updates management thread.
		*/
		static const std::condition_variable& priorityManagerThreadCV_C();

		/**
		* @brief Returns the condition variable associated with the priority new chunks meshes list.
		*/
		static const std::condition_variable& priorityNewChunkMeshesCV_C();

		/**
		* @brief Get the chunk specified at the given chunk-grid coordinates.
		* @param chunkPos The given chunk-grid coordinates.
		* @return The specified chunk.
		*/
		static const chunk* getChunkC(const vec3& chunkPos);

		static neighborsInfo* getChunkNeighborInfo(const vec3& chunkPos);


		// Modifiers.

		/**
		* @brief Set the number of chunks to compute in the X and Z axes.
		*/
		static void setNChunksToCompute(unsigned int nChunksToCompute);

		/**
		* @brief Set the block ID of a specfied block position.
		* 'triggerRenderingSync' tells the chunk manager system to perform a forcible
		* synchronization between all threads related to chunk rendering in order
		* to update the chunk's mesh that was modified by this method.
		*/
		static const block& setBlock(const vec3& blockPos, const block& blockID);

		/**
		* @brief Set the block ID of a specfied block position.
		* 'triggerRenderingSync' tells the chunk manager system to perform a forcible
		* synchronization between all threads related to chunk rendering in order
		* to update the chunk's mesh that was modified by this method.
		*/
		static const block& setBlock(int x, int y, int z, const block& blockID);

		/**
		* @brief Select a chunk with the specified chunk position.
		* WARNING. Not meant for use in AI mode.
		*/
		static chunk* selectChunk(int chunkX, int chunkY, int chunkZ);

		/**
		* @brief Select a chunk with the specified chunk position.
		* WARNING. Not meant for use in AI mode.
		*/
		static chunk* selectChunk(const vec3& chunkPos);

		/**
		* @brief Select a chunk with the specified block position by
		* converting the global position cords x, y and z into chunk position.
		* WARNING. Not meant for use in AI mode.
		*/
		static chunk* selectChunkByChunkPos(int x, int y, int z);

		/**
		* @brief Select a chunk with the specified global position.
		* WARNING. Not meant for use in AI mode.
		*/
		static chunk* selectChunkByRealPos(const vec3& pos);

		/**
		* @brief Select the neighbor -X chunk for the chunk with the specified chunk position
		* WARNING. Not meant for use in AI mode.
		*/
		static chunk* neighborMinusX(const vec3& chunkPos);

		/**
		* @brief Select the neighbor +X chunk for the chunk with the specified chunk position
		* WARNING. Not meant for use in AI mode.
		*/
		static chunk* neighborPlusX(const vec3& chunkPos);

		/**
		* @brief Select the neighbor -Y chunk for the chunk with the specified chunk position
		* WARNING. Not meant for use in AI mode.
		*/
		static chunk* neighborMinusY(const vec3& chunkPos);

		/**
		* @brief Select the neighbor +Y chunk for the chunk with the specified chunk position
		* WARNING. Not meant for use in AI mode.
		*/
		static chunk* neighborPlusY(const vec3& chunkPos);

		/**
		* @brief Select the neighbor -Z chunk for the chunk with the specified chunk position
		* WARNING. Not meant for use in AI mode.
		*/
		static chunk* neighborMinusZ(const vec3& chunkPos);

		/**
		* @brief Select the neighbor +Z chunk for the chunk with the specified chunk position
		* WARNING. Not meant for use in AI mode.
		*/
		static chunk* neighborPlusZ(const vec3& chunkPos);

		/**
		* @brief Returns the mutex that guards the registered chunks dictionary.
		* WARNING. Not meant for use in AI mode.
		*/
		static std::recursive_mutex& chunksMutex();

		/**
		* @brief Returns the mutex associated with the chunk management thread.
		* WARNING. Not meant for use in AI mode.
		*/
		static std::mutex& managerThreadMutex();

		/**
		* @brief Returns the mutex associated with the priority chunk updates management thread.
		* WARNING. Not meant for use in AI mode.
		*/
		static std::mutex& priorityManagerThreadMutex();

		/**
		* @brief Returns the condition variable associated with the chunk management thread.
		* WARNING. Not meant for use in AI mode.
		*/
		static std::condition_variable& managerThreadCV();

		/**
		* @brief Returns the condition variable associated with the chunk priority updates management thread.
		*/
		static std::condition_variable& priorityManagerThreadCV();

		/**
		* @brief Returns the condition variable associated with the priority new chunks meshes list.
		*/
		static std::condition_variable& priorityNewChunkMeshesCV();

		/**
		* @brief Locks the calling thread until:
		* - Minimal terrain has been loaded around the player (infinite world).
		* - The entire level has been loaded (finite world).
		*/
		static void waitInitialTerrainLoaded();

		/**
		* @brief Unloads the chunk at chunk position 'chunkPos', pushing it into a free chunks deque
		* to be reused later for another chunk position of the world. Returns the number of chunks
		* that were converted into frontier chunks because of this operation.
		*/
		static void unloadFrontierChunk(const vec3& chunkPos);

		static void undoNeighborInfo(chunk& c);

		/**
		* @brief Method called by the chunk management thread to use with infinite world types.
		* It coordinates all meshing threads and manages the chunk unloading process
		* in order to prevent any race condition between said threads, among other things
		* such as synchronization and data transfering with the rendering thread.
		* NOTE. This method does not handle the chunk priority updates.
		*/
		static void manageChunks();

		/**
		* @brief Similar to chunkManager::manageChunks() but only for priority chunk updates.
		*/
		static void manageChunkPriorityUpdates();

		/**
		* @brief Set 'newName' to "" to clear the opened terrain file name.
		*/
		static void openedTerrainFileName(const std::string& newName);

		/**
		* @brief Load and mesh the chunk in the specified chunk coordinates if visible.
		* Returns true if the chunk could was visible, loaded and meshed or false
		* otherwise.
		*/
		static bool ensureChunkIfVisible(const vec3& chunkPos);

		/**
		* @brief Load and mesh the chunk in the specified chunk coordinates if visible.
		* Returns true if the chunk could was visible, loaded and meshed or false
		* otherwise.
		*/
		static bool ensureChunkIfVisible(int x, int y, int z);

		/**
		* @brief Serialize the chunk's data in order to save it into auxiliary memory.
		*/
		static std::string serializeChunk(chunk* c);

		/**
		* @brief Deserialize the given chunk data into the given chunk.
		* @param c The given chunk to fill with the given chunk data.
		* @param data The data to fill the given chunk with.
		*/
		static void deserializeChunk(chunk* c, const std::string& data);

		/**
		* @brief Load chunk at specified chunk coordinates and return a pointer to its corresponding object.
		* If it is already loaded, it only returns its corresponding object.
		* NOTE. If it founds serialized data corresponding to this chunk, it will fill the chunk
		* with said data instead of using the world generator.
		* WARNING. DOES NOT CHECK IF THERE IS ALREADY A CHUNK AT THE SPECIFIED POSITION.
		*/
		static chunk* loadChunk(const vec3& chunkPos);

		/**
		* @brief Issue a job related to chunk processing.
		* The job will be executed on another thread and will lock the chunk's mutexes that
		* are required.
		* @param pushJobBack. Whether to insert the job at the back of the queue (true) or at the beginning (false).
		*/
		static void issueChunkMeshJob(chunkJobType type, void* data, bool pushJobBack = true);

		/** 
		* @brief Used on chunkManager::onUnloadAsFrontier to update the neighbor
		* chunks of a frontier chunk that was unloaded.
		*/
		static void onUnloadAsFrontier(const vec3& chunkPos);

		/**
		* @brief Convert the specified chunk into a frontier chunk if it is not.
		*/
		static void addFrontier(chunk* chunk);

		/**
		* @brief Renews the mesh of the specified chunk and marks it if it is ready to be drawn.
		* Sets the chunk's status to MESHED.
		*/
		static void remesh(chunk* c, bool isPriorityUpdate);

		/**
		* @brief Renews the mesh of the specified chunk and marks it if it is ready to be drawn.
		*/
		static void renewMesh(const vec3& chunkPos, bool isPriorityUpdate);

		/**
		* @brief Returns the onChunkLoad chunkEvent associated with the chunk management system.
		*/
		static chunkEvent& onChunkLoad();

		/**
		* @brief Returns the onChunkUnload chunkEvent associated with the chunk management system.
		*/
		static chunkEvent& onChunkUnload();

		/**
		* @brief Get the chunk specified at the given chunk-grid coordinates.
		* @param chunkPos The given chunk-grid coordinates.
		* @return The specified chunk.
		*/
		static chunk* getChunk(const vec3& chunkPos);

		/**
		* @brief Get the mutex that provides mutual exclusion for the dictionary of chunk's neighborInfo objects.
		* @return The mutex that provides mutual exclusion for the dictionary of chunk's neighborInfo objects.
		*/
		static std::mutex& chunkNeighborsInfoMutex();

		static void passLightToNeighbor(const blockLightMod& floodLight, basicVec3& pos, const vec3& neighborOffset, const vec3& chunkPos);

		static std::shared_ptr<neighborsInfo> getOrCreateChunkNeighborInfo(const vec3& chunkPos);


		// Clean Up.

		static void clearChunks();

		/**
		* @brief Only cleans any resources like chunks created but it does not de-initialise
		* the chunk management system.
		*/
		static void clear();

		/**
		* @brief Frees any memory allocated in the process of generating the world, like
		* all the Chunk objects created to load it.
		*/
		static void reset();

	private:

		/*
		Nested classes/types/enums.
		*/

		/**
		* @brief Comparator function object class used to sort a collection of chunks with how
		* closer they are to the player.
		*/
		class closestChunk {

		public:

			/**
			* @brief NOTE. Per C++ standards, if this operator returns true it means that chunkPos1 goes before
			* chunkPos2 in a properly sorted collection.
			* Used to sort a collection of chunk positions with how
			* closer they are to the player. Returns true if chunkPos1's distance to the player
			* in chunk coordinates is less than the distance of chunkPos2 to the player in chunk coordinates
			* or false otherwise.
			* In case they have the same distance to the player in chunk coordinates, it returns true if chunkPos1 < chunkPos2.
			*/
			inline bool operator()(const vec3& chunkPos1, const vec3& chunkPos2) const {

				return distanceToPlayer(chunkPos1) < distanceToPlayer(chunkPos2);

			}

		};

		/**
		* Brief Predicate Function that is used to determine whether a chunk-coordiante position is inside
		* the player's render distance or not.
		*/
		class notInRenderDistance {

		public:

			// Modifiers.

			/**
			* @brief Returns true if the specified chunk-coordinate position is inside the player's
			* render distance or false otherwise.
			*/
			inline bool operator()(const vec3& chunkPos) const {

				return !chunkInRenderDistance(chunkPos);

			}

		};

		/*
		Attributes.
		*/

		static bool initialised_;
		static bool chunkJobSystemOverloaded_;
		static int nChunksToCompute_;

		static std::atomic<bool> clearChunksFlag_;
		static std::atomic<bool> priorityUpdatesRemaining_;

		/*
		Used to force all meshing threads to synchronize with
		the rendering thread when a high priority
		chunk update is issued and it's imperative than the update
		made is reflected in the rendering thread as quickly as possible.
		*/
		static std::atomic<bool> waitInitialTerrainLoaded_;

		// Chunks that are loaded by the player.
		static std::unordered_map<vec3, chunk*> clientChunks_; 

		// Chunks that are not loaded by the player
		// (for example, chunks loaded by an AI agent 
		// that is mining blocks far away from it). TODO <- HACER QUE LOS CHUNKS QUE NO SEAN CARGADOS POR JUGADOR SE METAN AQUI. ESTOS CHUNKS NO SON RENDERIZADOS A NO SER QUE EL PLAYER ESTÉ, EN CUYO CASO ESTARÍA SOLO COMO CLIENT_CHUNK.
													// ASI, SI SE PIDE UN CHUNK POR UN METODO GENÉRICO, SI NO SE ENCUENTRA ESE CHUNK EN CLIENT CHUNKS, SE CARGA COMO SIMULATED CHUNK.
		static std::unordered_map<vec3, chunk*> simulatedChunks_; 

		static std::unordered_map<vec3, chunkRenderingData>* chunkMeshesUpdated_; // Chunk meshes generated by renewMesh()
		static std::unordered_map<vec3, chunkRenderingData>* chunkMeshesWrite_; // Chunk meshes to be passed to the rendering thread on next synchronization between this one and the chunk meshing threads.
		static std::unordered_map<vec3, chunkRenderingData>* chunkMeshesRead_; // Chunk meshes being used by the rendering thread ONLY.

		// NEXT. HAY QUE PONER VERSIÓN DE PRIORITY.
		static std::unordered_map<vec3, chunkVBOoperation>* chunkVBOoperationsWrite_;
		static std::unordered_map<vec3, chunkVBOoperation>* chunkVBOoperationsRead_;
		static std::mutex chunkVBOoperationsMutex_;

		static std::list<chunk*> newChunkMeshes_;
		static std::list<chunk*> priorityNewChunkMeshes_;
			                      
		static std::mutex managerThreadMutex_,
						  priorityManagerThreadMutex_,
						  priorityNewChunkMeshesMutex_,
						  priorityUpdatesRemainingMutex_,
						  loadingTerrainMutex_;
		static std::recursive_mutex newChunkMeshesMutex_,
						            chunksMutex_;
		static std::condition_variable managerThreadCV_,
									   priorityManagerThreadCV_,
									   priorityNewChunkMeshesCV_,
									   loadingTerrainCV_;
		static std::condition_variable_any priorityUpdatesRemainingCV_;

		static std::unordered_map<unsigned int, std::unordered_map<vec3, bool>> AIChunkAvailable_;
		static std::unordered_map<unsigned int, std::unordered_map<vec3, chunk*>> AIagentChunks_;
		static unsigned int selectedAIWorld_;
		static bool originalWorldAccess_;

		static vec3 playerChunkPosCopy_; // Copy of the last value of the player's position in chunk coordinates.

		static std::list<vec3> frontierChunks_;
		static std::unordered_map<vec3, std::list<vec3>::iterator> frontierChunksSet_;
		static std::list<vec3>::iterator frontierIt_;

		static closestChunk closestChunk_;

		static threadPool* chunkTasks_;
		static threadPool* priorityChunkTasks_;

		static std::unordered_map<vec3, unsigned int> currentJobsPerChunk_;

		static atomicRecyclingPool<job>* loadChunkJobs_;
		static atomicRecyclingPool<chunk> chunksPool_;

		static chunkEvent onChunkLoad_;
		static chunkEvent onChunkUnload_;

		static std::mutex chunkNeighborsInfoMutex_;
		static std::unordered_map<vec3, std::shared_ptr<neighborsInfo>> chunkNeighborsInfo_;

		static chunkVertexBuffer* vbo_;

		static std::string openedTerrainFileName_; // TODO. DEPRECEATED. REPLACE WITH THE WORLD SYSTEM EQUIVALENT NAMED 'currentWorldPath_'.

		/*
		Methods.
		*/

		static const block& getBlockOGWorld_(int posX, int posY, int posZ);

		static void pushNewChunkMesh(bool isPriorityUpdate, chunk* c, std::size_t meshSize);

		

		/*
		Job methods.
		*/
		static void loadChunkJob(void* data);

		static void onLoadChunkJobFinish(chunk* c);

		static void loadChunkJobPass2(void* data);

		static void remeshChunkJob(void* data);

		static void unloadAndSaveChunkJob(void* data);

		static void priorityRemeshChunkJob(void* data);

	};

	inline bool chunkManager::initialised() {
	
		return initialised_;
	
	}

	inline const std::unordered_map<vec3, chunk*>& chunkManager::chunks() {

		return clientChunks_;

	}

	inline std::unordered_map<vec3, chunkRenderingData> const * chunkManager::drawableChunksRead() {

		return chunkMeshesRead_;

	}

	inline std::unordered_map<vec3, chunkVBOoperation> const * chunkManager::chunkVBOoperationsRead() {
	
		return chunkVBOoperationsRead_;
	
	}

	inline unsigned int chunkManager::nChunksToCompute() {

		return nChunksToCompute_;

	}

	inline const block& chunkManager::getBlock(const vec3& blockPos) {

		return getBlock(blockPos.x, blockPos.y, blockPos.z);

	}

	inline std::vector<const block*> chunkManager::getBlocksBox(const vec3& pos1, const vec3& pos2) {

		// std::vector has move semantics. This allows us to avoid the unnecessary copies that would otherwise be made here.
		return getBlocksBox(pos1.x, pos1.y, pos1.z, pos2.x, pos2.y, pos2.z);

	}

	inline bool chunkManager::isInWorld(const vec3& blockPos) {
	
		return isInWorld(blockPos.x, blockPos.y, blockPos.z);
	
	}

	inline bool chunkManager::chunkInRenderDistance(const chunk* chunk) {

		return chunkInRenderDistance(chunk->chunkPos());

	}

	inline bool chunkManager::chunkInRenderDistance(int chunkPosX, int chunkPosY, int chunkPosZ) {
	
		return chunkInRenderDistance(vec3{ chunkPosX, chunkPosY, chunkPosZ });
	
	}

	inline bool chunkManager::isInWorld(int x, int y, int z) {

		return x >= -nChunksToCompute_ * CHUNK_SIZE && x < (nChunksToCompute_ - 1) * CHUNK_SIZE &&
			   y >= -yChunksRange * CHUNK_SIZE && y < (yChunksRange - 1) * CHUNK_SIZE&&
			   z >= -nChunksToCompute_ * CHUNK_SIZE && z < (nChunksToCompute_ - 1) * CHUNK_SIZE;
	
	}

	inline bool chunkManager::isChunkInWorld(int chunkX, int chunkY, int chunkZ) {

		return isChunkInWorld(vec3{ chunkX, chunkY, chunkZ });

	}

	inline chunkStatus chunkManager::getChunkLoadLevel(int chunkX, int chunkY, int chunkZ) {

		return getChunkLoadLevel(vec3{ chunkX, chunkY, chunkZ });

	}

	inline const std::string& chunkManager::openedTerrainFileName() {

		return openedTerrainFileName_;
		
	}

	inline const chunkEvent& chunkManager::onChunkLoadC() {
	
		return onChunkLoad_;
	
	}

	inline const chunkEvent& chunkManager::onChunkUnloadC() {
	
		return onChunkUnload_;
	
	}

	inline unsigned int chunkManager::nMaxChunksToCompute() {

		return nChunksToCompute_ * 2 * nChunksToCompute_ * 2 * totalYChunks;

	}

	inline unsigned int chunkManager::nMaxChunkVertsToCompute() {

		return nBlocksChunk * 36; // 6 faces * 6 verticesPerFace = maximum 36 vertices per block.
	
	}

	inline const std::condition_variable& chunkManager::priorityManagerThreadCV_C() {
	
		return priorityManagerThreadCV_;
	
	}

	inline const std::condition_variable& chunkManager::priorityNewChunkMeshesCV_C() {
	
		return priorityNewChunkMeshesCV_;
	
	}

	inline const chunk* chunkManager::getChunkC(const vec3& chunkPos) {
	
		std::unique_lock<std::recursive_mutex> lock(chunksMutex_);
		return clientChunks_.contains(chunkPos) ? clientChunks_[chunkPos] : nullptr;

	}

	inline chunk* chunkManager::selectChunk(int x, int y, int z) {

		return selectChunk(vec3{ x, y, z });

	}
	
	inline const block& chunkManager::setBlock(const vec3& pos, const block& blockID) {

		return setBlock(pos.x, pos.y, pos.z, blockID);

	}

	inline std::recursive_mutex& chunkManager::chunksMutex() {

		return chunksMutex_;

	}

	inline std::mutex& chunkManager::managerThreadMutex() {

		return managerThreadMutex_;

	}

	inline std::mutex& chunkManager::priorityManagerThreadMutex() {

		return priorityManagerThreadMutex_;

	}

	inline std::condition_variable& chunkManager::managerThreadCV() {

		return managerThreadCV_;

	}

	inline std::condition_variable& chunkManager::priorityManagerThreadCV() {

		return priorityManagerThreadCV_;

	}

	inline std::condition_variable& chunkManager::priorityNewChunkMeshesCV() {
	
		return priorityNewChunkMeshesCV_;
	
	}

	inline bool chunkManager::ensureChunkIfVisible(int chunkPosX, int chunkPosY, int chunkPosZ) {

		return ensureChunkIfVisible(vec3{ chunkPosX, chunkPosY, chunkPosZ });

	}

	inline chunkEvent& chunkManager::onChunkLoad() {
	
		return onChunkLoad_;
	
	}

	inline chunkEvent& chunkManager::onChunkUnload() {

		return onChunkUnload_;

	}

	inline chunk* chunkManager::getChunk(const vec3& chunkPos) {

		std::unique_lock<std::recursive_mutex> lock(chunksMutex_);
		return clientChunks_.contains(chunkPos) ? clientChunks_[chunkPos] : nullptr;

	}

	inline std::mutex& chunkManager::chunkNeighborsInfoMutex() {
	
		return chunkNeighborsInfoMutex_;
	
	}

	inline void chunkManager::clearChunks() {

		clearChunksFlag_ = true;

	}

}

#endif