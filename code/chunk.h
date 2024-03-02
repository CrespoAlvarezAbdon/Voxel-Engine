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
#include <hash.hpp>

#include "atomicRecyclingPool.h"
#include "block.h"
#include "definitions.h"
#include "event.h"
#include "listener.h"
#include "texture.h"
#include "threadPool.h"
#include "shader.h"
#include "model.h"
#include "palette.h"
#include "vec.h"
#include "vertex.h"
#include "vertexBuffer.h"
#include "vertexBufferLayout.h"
#include "utilities.h"
#include "time.h"

#if GRAPHICS_API == OPENGL

#include <GL/glew.h>
#include <glm.hpp>

#endif


namespace VoxelEng {

	/////////////////////////
	//Forward declarations.//
	/////////////////////////

	class chunkManager;


	/////////////////
	//Enum classes.//
	/////////////////

	/**
	* @brief The different stages that a chunk has during its lifetime.
	*/
	enum class chunkStatus { NOTLOADED = 0, BASICTERRAIN = 1, DECORATED = 2};

	enum class chunkJobType { NONE = 0, LOAD = 1, ONLYREMESH = 2, UNLOADANDSAVE = 3, PRIORITYREMESH = 4};


	////////////
	//Structs.//
	////////////

	/**
	* @brief Wraps up data used to render a chunk.
	*/
	struct chunkRenderingData {

		vec3 chunkPos;
		model vertices; // prueba a poner esto como model a secas y donde ponga delete vertices simplemente recrear el objeto

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
		chunk(const chunk& source);


		// Observers.

		/**
		* @brief Returns true if the static members of this class are initialised
		* or false otherwise.
		*/
		static bool initialised();

		/**
		* @brief Returns the pointer to the first element of the chunk's block array.
		*/
		const void* blocks() const;

		/**
		* @brief Get the block at the specified chunk-local coordinates.
		*/
		const block& getBlock(GLbyte x, GLbyte y, GLbyte z);

		/**
		* @brief Get the block at the specified chunk-local coordinates.
		*/
		const block& getBlock(const vec3& inChunkPos);

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
		* @brief Returns true if this chunk's mesh needs
		* to be regenerated or false otherwise.
		*/
		bool needsRemesh() const;

		/**
		* @brief Returns the chunk's status.
		*/
		chunkStatus status() const;

		/**
		* @brief Get the number of non-null blocks (blocks with ID != 0) that exist in the chunk.
		*/
		unsigned short nBlocks() const;

		/**
		* @brief Returns the number of non-null blocks in the chunk's +X edge.
		*/
		unsigned short nBlocksPlusX() const;

		/**
		* @brief Returns the number of non-null blocks in the chunk's -X edge.
		*/
		unsigned short nBlocksMinusX() const;

		/**
		* @brief Returns the number of non-null blocks in the chunk's +Y edge.
		*/
		unsigned short nBlocksPlusY() const;

		/**
		* @brief Returns the number of non-null blocks in the chunk's -Y edge.
		*/
		unsigned short nBlocksMinusY() const;

		/**
		* @brief Returns the number of non-null blocks in the chunk's +Z edge.
		*/
		unsigned short nBlocksPlusZ() const;

		/**
		* @brief Returns the number of non-null blocks in the chunk's -Z edge.
		*/
		unsigned short nBlocksMinusZ() const;

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
		* @brief Returns true if the chunk is visible by the player or false otherwise.
		*/
		bool isVisible() const;

		/**
		* @brief Returns the number of neighbors that this chunk has.
		*/
		unsigned int nNeighbors() const;


		// Modifiers.

		/**
		* @brief Returns the pointer to the first element of the chunk's block array.
		*/
		void* blocks();

		/**
		* @brief Returns the pointer to the first element of the x+ neighbor chunk's block array.
		*/
		void* neighborBlocksPlusX();

		/**
		* @brief Returns the pointer to the first element of the x- neighbor chunk's block array.
		*/
		void* neighborBlocksMinusX();

		/**
		* @brief Returns the pointer to the first element of the y+ neighbor chunk's block array.
		*/
		void* neighborBlocksPlusY();

		/**
		* @brief Returns the pointer to the first element of the y- neighbor chunk's block array.
		*/
		void* neighborBlocksMinusY();

		/**
		* @brief Returns the pointer to the first element of the z+ neighbor chunk's block array.
		*/
		void* neighborBlocksPlusZ();

		/**
		* @brief Returns the pointer to the first element of the z- neighbor chunk's block array.
		*/
		void* neighborBlocksMinusZ();

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
		* chunk's generation process or otherwise.
		* WARNING. For world generators that use this method: 'modification' must be set to false.
		*/
		const block& setBlock(unsigned int linearIndex, const block& block, bool modification = true);

		/**
		* @brief Set a neighbor block.
		* A neighbor block is a copy of a block that is bordering this chunk. This copy is stored for mesh optimization
		* purposes.
		*/
		void setBlockNeighbor(unsigned int firstIndex, unsigned int secondIndex, blockViewDir neighbor, const block& block);

		/**
		* @brief Returns the chunk's chunk position.
		*/
		vec3& chunkPos();

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
		* @brief Set if this chunk's terrain has been modified and it's mesh needs
		* to be regenerated.
		*/
		void needsRemesh(bool newValue);

		/**
		* @brief Returns true if this chunk has been modified
		* since being generated or loaded from disk or
		* false otherwise.
		*/
		bool& modified();

		/**
		* @brief Regenerate the chunk's mesh. Returns true if the mesh contains vertices or false if it is empty.
		*/
		bool renewMesh();

		/**
		* @brief The chunk's block data will be filled with null blocks, leaving the chunk "empty of blocks".
		*/
		void makeEmpty();

		/**
		* @brief Set the chunk's status.
		*/
		void status(chunkStatus level);

		/**
		* @brief Set the number of non-null blocks (blocks with ID != 0) that exist in the chunk.
		*/
		void nBlocks(unsigned short newValue);

		/**
		* @brief Set the number of non-null blocks in the chunk's +X edge.
		*/
		void nBlocksPlusX(unsigned short newValue);

		/**
		* @brief Set the number of non-null blocks in the chunk's -X edge.
		*/
		void nBlocksMinusX(unsigned short newValue);

		/**
		* @brief Set the number of non-null blocks in the chunk's +Y edge.
		*/
		void nBlocksPlusY(unsigned short newValue);

		/**
		* @brief Set the number of non-null blocks in the chunk's -Y edge.
		*/
		void nBlocksMinusY(unsigned short newValue);

		/**
		* @brief Set the number of non-null blocks in the chunk's +Z edge.
		*/
		void nBlocksPlusZ(unsigned short newValue);

		/**
		* @brief Set the number of non-null blocks in the chunk's -Z edge.
		*/
		void nBlocksMinusZ(unsigned short newValue);

		/**
		* @brief Executed when the frontier chunk is unloaded.
		* Checks if the neighbors of this chunk become frontier chunks
		* after this one stops being loaded.
		*/
		void onUnloadAsFrontier();

		/**
		* @brief Set the number of neighbors that this chunk has.
		*/
		unsigned int& nNeighbors();


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

		// Serializable data.
		palette<unsigned short, unsigned int> palette_;
		std::unordered_map<unsigned short, unsigned short> paletteCount_;
		std::unordered_set<unsigned short> freeLocalIDs_;
		unsigned short blocksLocalIDs[SCX][SCY][SCZ];
		unsigned short neighborBlocksPlusX_[SCY][SCZ];
		unsigned short neighborBlocksMinusX_[SCY][SCZ];
		unsigned short neighborBlocksPlusY_[SCX][SCZ];
		unsigned short neighborBlocksMinusY_[SCX][SCZ];
		unsigned short neighborBlocksPlusZ_[SCX][SCY];
		unsigned short neighborBlocksMinusZ_[SCX][SCY];

		bool modified_;
		std::atomic<bool> needsRemesh_;
		std::atomic<short> nBlocks_,
						   nBlocksPlusX_,
						   nBlocksMinusX_,
						   nBlocksPlusY_,
						   nBlocksMinusY_,
						   nBlocksPlusZ_,
						   nBlocksMinusZ_;

		unsigned int nNeighbors_;

		std::atomic<chunkStatus> loadLevel_;

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

	};

	inline const void* chunk::blocks() const {

		return blocksLocalIDs;

	}

	inline bool chunk::initialised() {
	
		return initialised_;
	
	}

	inline const block& chunk::getBlock(const vec3& inChunkPos) {

		return getBlock(inChunkPos.x, inChunkPos.y, inChunkPos.z);

	}

	inline GLbyte chunk::x() const {

		return renderingData_.chunkPos.x;

	}

	inline GLbyte chunk::y() const {

		return renderingData_.chunkPos.y;

	}

	inline GLbyte chunk::z() const {

		return renderingData_.chunkPos.z;

	}

	inline const vec3& chunk::chunkPos() const {

		return renderingData_.chunkPos;

	}

	inline vec3& chunk::chunkPos() {

		return renderingData_.chunkPos;
	
	}

	inline const vec3& chunk::pos() const {

		return vec3{ (float)renderingData_.chunkPos.x * SCX, (float)renderingData_.chunkPos.y * SCY, (float)renderingData_.chunkPos.z * SCZ };

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

	inline chunkStatus chunk::status() const {

		return loadLevel_;

	}

	inline unsigned short chunk::nBlocks() const {

		return nBlocks_;

	}

	inline unsigned short chunk::nBlocksPlusX() const {
	
		return nBlocksPlusX_;
	
	}

	inline unsigned short chunk::nBlocksMinusX() const {

		return nBlocksMinusX_;

	}

	inline unsigned short chunk::nBlocksPlusY() const {

		return nBlocksPlusY_;

	}

	inline unsigned short chunk::nBlocksMinusY() const {

		return nBlocksMinusY_;

	}

	inline unsigned short chunk::nBlocksPlusZ() const {

		return nBlocksPlusZ_;

	}

	inline unsigned short chunk::nBlocksMinusZ() const {

		return nBlocksMinusZ_;

	}

	inline bool chunk::isEmptyBlock(GLbyte x, GLbyte y, GLbyte z) const {
	
		return blocksLocalIDs[x][y][z] == 0;
	
	}

	inline bool chunk::isEmptyBlock(const vec3& inChunkPos) const {
	
		return isEmptyBlock(inChunkPos.x, inChunkPos.y, inChunkPos.z);
	
	}

	inline bool chunk::isVisible() const {
	
		// TODO. ADD CONDITIONS TO TAKE INTO ACCOUNT FUSTRUM CULLING OTHER ALGORITHMS TO AVOID RENDERING UNSEEABLE CHUNKS.
		return (nBlocks_ > 0 || nBlocks_ <= nBlocksChunk);
	
	}

	inline unsigned int chunk::nNeighbors() const {
	
		return nNeighbors_;
	
	}

	inline void* chunk::blocks() {
	
		return blocksLocalIDs;
	
	}

	inline void* chunk::neighborBlocksPlusX() {

		return neighborBlocksPlusX_;

	}

	inline void* chunk::neighborBlocksMinusX() {

		return neighborBlocksMinusX_;

	}

	inline void* chunk::neighborBlocksPlusY() {

		return neighborBlocksPlusY_;

	}

	inline void* chunk::neighborBlocksMinusY() {

		return neighborBlocksMinusY_;

	}

	inline void* chunk::neighborBlocksPlusZ() {

		return neighborBlocksPlusZ_;

	}

	inline void* chunk::neighborBlocksMinusZ() {

		return neighborBlocksMinusZ_;

	}

	inline palette<unsigned short, unsigned int>& chunk::getPalette() {
	
		return palette_;
	
	}

	inline std::unordered_map<unsigned short, unsigned short>& chunk::getPaletteCount() {
	
		return paletteCount_;
	
	}

	inline const block& chunk::setBlock(const vec3& chunkRelPos, const block& b, bool modification) {

		return setBlock(chunkRelPos.x, chunkRelPos.y, chunkRelPos.z, b, modification);

	}

	inline const block& chunk::setBlock(unsigned int linearIndex, const block& b, bool modification) {

		return setBlock(linearToVec3(linearIndex, SCY, SCZ), b, modification);

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

	inline bool& chunk::modified() {

		return modified_;

	}

	inline void chunk::status(chunkStatus level) {

		loadLevel_ = level;

	}

	inline void chunk::nBlocks(unsigned short newValue) {

		nBlocks_ = newValue;

	}

	inline void chunk::nBlocksPlusX(unsigned short newValue) {

		nBlocksPlusX_ = newValue;

	}

	inline void chunk::nBlocksMinusX(unsigned short newValue) {

		nBlocksMinusX_ = newValue;

	}

	inline void chunk::nBlocksPlusY(unsigned short newValue) {

		nBlocksPlusY_ = newValue;

	}

	inline void chunk::nBlocksMinusY(unsigned short newValue) {

		nBlocksMinusY_ = newValue;

	}

	inline void chunk::nBlocksPlusZ(unsigned short newValue) {

		nBlocksPlusZ_ = newValue;

	}

	inline void chunk::nBlocksMinusZ(unsigned short newValue) {

		nBlocksMinusZ_ = newValue;

	}

	inline unsigned int& chunk::nNeighbors() {
	
		return nNeighbors_;
	
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


	// 'loadChunkJob' class.

	/**
	* @brief Task that loads a chunk either by reading its data from disk
	* or by generating it using the currently selected world generator.
	*/
	class chunkJob : public job {
	public:

		// Constructors.

		/**
		* @brief Default class constructor.
		*/
		chunkJob();


		// Modifiers.

		/**
		* @brief Change the attributes used in the construction of this object.
		* WARNING. 'type' most NOT BE chunkJobType::REMESHNEIGBHOR. Method setRemeshNeighborAttributes is specialized
		* for that job.
		*/
		void setAttributes(chunk* c, chunkJobType type, atomicRecyclingPool<chunkJob>* pool, std::condition_variable* priorityNewChunkMeshesCV, atomicRecyclingPool<chunk>* chunkPool);


	private:

		/*
		Methods.
		*/

		void process();


		/*
		Attributes.
		*/

		chunk* chunk_;
		chunkJobType type_;
		atomicRecyclingPool<chunkJob>* pool_;
		std::condition_variable* priorityNewChunkMeshesCV_;
		atomicRecyclingPool<chunk>* chunkPool_;
	
	};

	inline chunkJob::chunkJob()
	: chunk_(nullptr),
	  type_(chunkJobType::NONE),
	  pool_(nullptr),
      priorityNewChunkMeshesCV_(nullptr),
	  chunkPool_(nullptr)
	{}

	
	// 'chunkManager' class.
	
	/**
	* @brief Used for managing the chunks' life cycle, level loading...
	*/
	class chunkManager {

	public:

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
		* @brief Returns the read-onlu copy of chunks' meshes.
		*/
		static std::unordered_map<vec3, model> const * drawableChunksRead();

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
		* @brief Generates only block data without the 3D graphics side.
		* Intended to be used when testing or training AI agents
		* without displaying the matches in the engine's graphical mode
		* to save CPU and GPU processing power.
		* This generated world is the original copy. Copies for each
		* AI agent will be generated in chunks as they are needed by
		* their respective agents.
		* If 'path' is equal to "" then a randomly generated world will
		* be created. Otherwise it will load de .terrain file
		* located at 'path' + ".terrain".
		*/
		static void generateAIWorld(const std::string& path = "");

		/**
		* @brief Get block and set block operations in the chunk manager system will now
		* be performed on the AI world/level of AI agent with ID 'individualID'.
		* AI mode must be turned on in the chunk manager system.
		* WARNING. This method is not thread safe.
		*/
		static void selectAIworld(unsigned int individualID);

		/**
		* @brief ONLY get block operations in the chunk manager system will now
		* be performed on the original copy of the level that is being used for the AI game.
		* AI mode must be turned on in the chunk manager system.
		* Set block operations will use the latest AI world selected (the one corresponding to
		* the AI agent with ID 0 by default).
		* WARNING. This method is not thread safe.
		*/
		static void selectOriginalWorld();

		/**
		* @brief Sets all copies of chunks owned by AI agents as they never existed (AIChunksAvailable[Any AI World copy][Any chunkCoord] will return false after this).
		*/
		static void resetAIChunks();

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
		* @brief Load chunk at specified chunk coordinates.
		* Returns a pointer to the loaded chunk.
		* NOTE. If it founds serialized data corresponding to this chunk, it will fill the chunk
		* with said data instead of using the world generator.
		* WARNING. DOES NOT CHECK IF THERE IS ALREADY A CHUNK AT THE SPECIFIED POSITION.
		*/
		static chunk* loadChunk(const vec3& chunkPos);

		/**
		* @brief Issue a job that will consists of rengerating the mesh of the specified chunk.
		* The job will be executed on another thread and will lock the chunk's mutexes that
		* are required.
		*/
		static void issueChunkMeshJob(chunk* c, chunkJobType type);

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
		*/
		static void renewMesh(chunk* chunk, bool isPriorityUpdate);

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
		Nested classes.
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


		// NEW THINGS START HERE

		static vec3 playerChunkPosCopy_; // Copy of the last value of the player's position in chunk coordinates.
		static std::list<vec3> frontierChunks_;
		static std::unordered_map<vec3, std::list<vec3>::iterator> frontierChunksSet_;
		static std::list<vec3>::iterator frontierIt_;
		static notInRenderDistance notInRenderDistance_;
		static closestChunk closestChunk_;
		static threadPool* chunkTasks_, 
						 * priorityChunkTasks_;
		static atomicRecyclingPool<chunkJob>* loadChunkJobs_;
		static atomicRecyclingPool<chunk> chunksPool_;
		static chunkEvent onChunkLoad_;
		static chunkEvent onChunkUnload_;
		static vertexBuffer* vbo_;
		static std::atomic<bool> clearChunksFlag_,
			                     priorityUpdatesRemaining_;

		// NEW THINGS END HERE

		/*
		Attributes.
		*/

		static bool initialised_;
		static int nChunksToCompute_;

		// Chunks that are loaded by the player.
		static std::unordered_map<vec3, chunk*> clientChunks_; 

		// Chunks that are not loaded by the player
		// (for example, chunks loaded by an AI agent 
		// that is mining blocks far away from it). NEXT <- HACER QUE LOS CHUNKS QUE NO SEAN CARGADOS POR JUGADOR SE METAN AQUI. ESTOS CHUNKS NO SON RENDERIZADOS A NO SER QUE EL PLAYER ESTÉ, EN CUYO CASO ESTARÍA SOLO COMO CLIENT_CHUNK.
													// ASI, SI SE PIDE UN CHUNK POR UN METODO GENÉRICO, SI NO SE ENCUENTRA ESE CHUNK EN CLIENT CHUNKS, SE CARGA COMO SIMULATED CHUNK.
		static std::unordered_map<vec3, chunk*> simulatedChunks_; 

		static std::unordered_map<vec3, model>* chunkMeshesUpdated_,
											  * chunkMeshesWrite_,
											  * chunkMeshesRead_;
		static std::list<chunk*> newChunkMeshes_,
								 priorityNewChunkMeshes_;
			                      
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

		/*
		Used to force all meshing threads to synchronize with
		the rendering thread when a high priority
		chunk update is issued and it's imperative than the update
		made is reflected in the rendering thread as quickly as possible.
		*/
		static std::atomic<bool> waitInitialTerrainLoaded_;

		static std::string openedTerrainFileName_; // TODO. DEPRECEATED. REPLACE WITH THE WORLD SYSTEM EQUIVALENT NAMED 'currentWorldPath_'.

		static std::unordered_map<unsigned int, std::unordered_map<vec3, bool>> AIChunkAvailable_;
		static std::unordered_map<unsigned int, std::unordered_map<vec3, chunk*>> AIagentChunks_;
		static unsigned int selectedAIWorld_;
		static bool originalWorldAccess_;


		/*
		Methods.
		*/
		static const block& getBlockOGWorld_(int posX, int posY, int posZ);

	};

	inline bool chunkManager::initialised() {
	
		return initialised_;
	
	}

	inline const std::unordered_map<vec3, chunk*>& chunkManager::chunks() {

		return clientChunks_;

	}

	inline std::unordered_map<vec3, model> const * chunkManager::drawableChunksRead() {

		return chunkMeshesRead_;

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

		return x >= -nChunksToCompute_ * VoxelEng::SCX && x < (nChunksToCompute_ - 1) * VoxelEng::SCX &&
			   y >= -yChunksRange * VoxelEng::SCY && y < (yChunksRange - 1) * VoxelEng::SCY &&
			   z >= -nChunksToCompute_ * VoxelEng::SCZ && z < (nChunksToCompute_ - 1) * VoxelEng::SCZ;
	
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

		return nBlocksChunk * 36; // TODO. PONER ESTO BONITO -> 6 caras * 6 vertPorCara = 36 vértices por bloque
	
	}

	inline const std::condition_variable& chunkManager::priorityManagerThreadCV_C() {
	
		return priorityManagerThreadCV_;
	
	}

	inline const std::condition_variable& chunkManager::priorityNewChunkMeshesCV_C() {
	
		return priorityNewChunkMeshesCV_;
	
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

	inline void chunkManager::clearChunks() {

		clearChunksFlag_ = true;

	}

}

#endif