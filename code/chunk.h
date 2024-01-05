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
	* @brief The different stages of the chunk's loading process.
	*/
	enum class chunkLoadLevel { NOTLOADED = 0, BASICTERRAIN = 1, DECORATED = 2 };


	////////////
	//Classes.//
	////////////

	/**
	* @brief Wraps up data used to render a chunk.
	*/
	struct chunkRenderingData {

		vec3 chunkPos;
		model vertices; // prueba a poner esto como model a secas y donde ponga delete vertices simplemente recrear el objeto

	};


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
		* @brief Returns the chunk's palette that maps the local block IDs with the global block IDs.
		*/
		const palette<unsigned short, unsigned int>& getPalette() const;

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
		* @brief Get the number of non-null blocks (blocks with ID != 0) that exist in the chunk.
		*/
		unsigned int getNBlocks();

		/**
		* @brief Returns true if this chunk's terrain has been modified and it's mesh needs
		* to be regenerated or false otherwise.
		*/
		bool changed() const;

		/**
		* @brief Returns the chunk's load level.
		*/
		chunkLoadLevel loadLevel() const;

		/**
		* @brief Returns the number of non-null blocks in the chunk's +X edge.
		*/
		short nBlocksPlusX() const;

		/**
		* @brief Returns the number of non-null blocks in the chunk's -X edge.
		*/
		short nBlocksMinusX() const;

		/**
		* @brief Returns the number of non-null blocks in the chunk's +Y edge.
		*/
		short nBlocksPlusY() const;

		/**
		* @brief Returns the number of non-null blocks in the chunk's -Y edge.
		*/
		short nBlocksMinusY() const;

		/**
		* @brief Returns the number of non-null blocks in the chunk's +Z edge.
		*/
		short nBlocksPlusZ() const;

		/**
		* @brief Returns the number of non-null blocks in the chunk's -Z edge.
		*/
		short nBlocksMinusZ() const;

		/**
		* @brief Returns the number of non-null blocks in the chunk's +X edge.
		*/
		chunk* neighborPlusX() const;

		/**
		* @brief Returns the number of non-null blocks in the chunk's -X edge.
		*/
		chunk* neighborMinusX() const;

		/**
		* @brief Returns the number of non-null blocks in the chunk's +Y edge.
		*/
		chunk* neighborPlusY() const;

		/**
		* @brief Returns the number of non-null blocks in the chunk's -Y edge.
		*/
		chunk* neighborMinusY() const;

		/**
		* @brief Returns the number of non-null blocks in the chunk's +Z edge.
		*/
		chunk* neighborPlusZ() const;

		/**
		* @brief Returns the number of non-null blocks in the chunk's -Z edge.
		*/
		chunk* neighborMinusZ() const;

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
		* @brief Sets the value of a block within the chunk.
		* The chunk is marked as dirty.
		* Returns the block that was there before.
		* 'triggerRenderingSync' tells the chunk manager system to perform a forcible
		* synchronization between all threads related to chunk rendering in order
		* to update the chunk's mesh that was modified by this method.
		*/
		const block& setBlock(GLbyte x, GLbyte y, GLbyte z, const block& block);

		/**
		* @brief Sets the value of a block within the chunk.
		* The chunk is marked as dirty.
		* Returns the old block.
		* 'triggerRenderingSync' tells the chunk manager system to perform a forcible
		* synchronization between all threads related to chunk rendering in order
		* to update the chunk's mesh that was modified by this method.
		*/
		const block& setBlock(const vec3& chunkRelPos, const block& block);

		/**
		* @brief Sets the value of a block within the chunk.
		* The chunk is marked as dirty.
		* Returns the old block.
		* 'triggerRenderingSync' tells the chunk manager system to perform a forcible
		* synchronization between all threads related to chunk rendering in order
		* to update the chunk's mesh that was modified by this method.
		*/
		const block& setBlock(unsigned int linearIndex, const block& block);

		/**
		* @brief Returns the chunk's chunk position.
		*/
		vec3& chunkPos();

		/**
		* @brief Returns the chunk's rendering data object.
		*/
		chunkRenderingData& renderingData();

		/**
		* @brief Locks the rendering data mutex.
		*/
		void lockRenderingDataMutex();

		/**
		* @brief Unlocks the rendering data mutex.
		* WARNING. Said lock must be unlock by the same thread that locked it previously.
		*/
		void unlockRenderingDataMutex();

		/**
		* @brief Set the number of non-null blocks in the chunk.
		*/
		void setNBlocks(unsigned int nBlocks);

		/**
		* @brief Returns the chunk's mutex that guards its block data.
		*/
		std::shared_mutex& blockDataMutex();

		/**
		* @brief Returns true if this chunk's terrain has been modified and it's mesh needs
		* to be regenerated or false otherwise.
		*/
		std::atomic<bool>& changed();

		/**
		* @brief Regenerate the chunk's mesh. Returns true if the mesh contains vertices or false if it is empty.
		*/
		bool renewMesh();

		/**
		* @brief The chunk's block data will be filled with null blocks, leaving the chunk "empty of blocks".
		*/
		void makeEmpty();

		/**
		* @brief Set the chunk's load level.
		*/
		void setLoadLevel(chunkLoadLevel level);

		/**
		* @brief Reuse a chunk object by assigning it a new chunk position and generating/loading
		* new terrain on it by previously cleaning any previous data.
		*/
		void regenChunk(bool empty, const vec3& chunkPos);

		/**
		* @brief Unlinks this chunk with all its neighbors.
		*/
		void unlinkNeighbors();

		/**
		* @brief Executed when the frontier chunk is unloaded.
		* Checks if the neighbors of this chunk become frontier chunks
		* after this one stops being loaded.
		*/
		int onUnloadAsFrontier();

		/**
		* @brief Update this chunks pointers to its neigbors according to its current chunk
		* position.
		*/
		void setNeighbors();

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

		// NEXT. IMPLEMENT THESE CHANGES.
		palette<unsigned short, unsigned int> palette_;
		std::unordered_map<unsigned short, unsigned short> paletteCount_;
		std::unordered_set<unsigned short> freeLocalIDs_;
		unsigned short blocksNew_[SCX][SCY][SCZ];

		std::atomic<bool> changed_;
		std::atomic<short> nBlocks_,
						   nBlocksPlusX_,
						   nBlocksMinusX_,
						   nBlocksPlusY_,
						   nBlocksMinusY_,
						   nBlocksPlusZ_,
						   nBlocksMinusZ_;
		unsigned int nNeighbors_;
		chunk* neighborPlusY_,
			 * neighborMinusY_,
			 * neighborPlusX_,
			 * neighborMinusX_,
			 * neighborPlusZ_,
			 * neighborMinusZ_;
		std::atomic<chunkLoadLevel> loadLevel_;
		chunkRenderingData renderingData_;
		std::recursive_mutex renderingDataMutex_;

		/*
		Used for reading the block data in a chunk.
		All meshing threads only read this data, so they access it
		in shared mode, while any thread that can alter this data
		must access it in exclusive/unique mode.
		*/
		std::shared_mutex blocksMutex_;

	};

	inline const void* chunk::blocks() const {

		return blocksNew_;

	}

	inline const palette<unsigned short, unsigned int>& chunk::getPalette() const {
	
		return palette_;
	
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

	inline unsigned int chunk::getNBlocks() {

		return nBlocks_;

	}

	inline bool chunk::changed() const {

		return changed_;

	}

	inline chunkLoadLevel chunk::loadLevel() const {

		return loadLevel_;

	}

	inline short chunk::nBlocksPlusX() const {
	
		return nBlocksPlusX_;
	
	}

	inline short chunk::nBlocksMinusX() const {

		return nBlocksMinusX_;

	}

	inline short chunk::nBlocksPlusY() const {

		return nBlocksPlusY_;

	}

	inline short chunk::nBlocksMinusY() const {

		return nBlocksMinusY_;

	}

	inline short chunk::nBlocksPlusZ() const {

		return nBlocksPlusZ_;

	}

	inline short chunk::nBlocksMinusZ() const {

		return nBlocksMinusZ_;

	}

	inline chunk* chunk::neighborPlusX() const {

		return neighborPlusX_;

	}

	inline chunk* chunk::neighborMinusX() const {

		return neighborMinusX_;

	}

	inline chunk* chunk::neighborPlusY() const {

		return neighborPlusY_;

	}

	inline chunk* chunk::neighborMinusY() const {

		return neighborMinusY_;

	}

	inline chunk* chunk::neighborPlusZ() const {

		return neighborPlusZ_;

	}

	inline chunk* chunk::neighborMinusZ() const {

		return neighborMinusZ_;

	}

	inline bool chunk::isEmptyBlock(GLbyte x, GLbyte y, GLbyte z) const {
	
		return blocksNew_[x][y][z] == 0;
	
	}

	inline bool chunk::isEmptyBlock(const vec3& inChunkPos) const {
	
		return isEmptyBlock(inChunkPos.x, inChunkPos.y, inChunkPos.z);
	
	}

	inline bool chunk::isVisible() const {
	
		return ((nBlocks_ > 0 || nBlocks_ <= nBlocksChunk) && 
			(neighborPlusZ_ && neighborPlusZ_->nBlocksMinusZ_ != nBlocksChunkEdge) ||
			(neighborMinusZ_ && neighborMinusZ_->nBlocksPlusZ_ != nBlocksChunkEdge) ||
			(neighborPlusY_ && neighborPlusY_->nBlocksMinusY_ != nBlocksChunkEdge) ||
			(neighborMinusY_ && neighborMinusY_->nBlocksPlusY_ != nBlocksChunkEdge) ||
			(neighborPlusX_ && neighborPlusX_->nBlocksMinusX_ != nBlocksChunkEdge) ||
			(neighborMinusX_ && neighborMinusX_->nBlocksPlusX_ != nBlocksChunkEdge));
	
	}

	inline unsigned int chunk::nNeighbors() const {
	
		return nNeighbors_;
	
	}

	inline const block& chunk::setBlock(const vec3& chunkRelPos, const block& b) {

		return setBlock(chunkRelPos.x, chunkRelPos.y, chunkRelPos.z, b);

	}

	inline const block& chunk::setBlock(unsigned int linearIndex, const block& b) {

		return setBlock(linearToVec3(linearIndex, SCY, SCZ), b);

	}

	inline chunkRenderingData& chunk::renderingData() {

		return renderingData_;

	}

	inline void chunk::lockRenderingDataMutex() {
	
		renderingDataMutex_.lock();
	
	}

	inline void chunk::unlockRenderingDataMutex() {
	
		renderingDataMutex_.unlock();
	
	}

	inline void chunk::setNBlocks(unsigned int nBlocks) {

		nBlocks_ = nBlocks;

	}

	inline std::shared_mutex& chunk::blockDataMutex() {

		return blocksMutex_;

	}

	inline std::atomic<bool>& chunk::changed() {

		return changed_;

	}

	inline void chunk::setLoadLevel(chunkLoadLevel level) {

		loadLevel_ = level;

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
	class meshChunkJob : public job {
	public:

		// Constructors.

		/**
		* @brief Default class constructor.
		*/
		meshChunkJob();

		/**
		* @brief Class constructor.
		*/
		meshChunkJob(chunk* chunk, atomicRecyclingPool<meshChunkJob>* pool, bool generateTerrain, bool isPriorityUpdate,
			std::condition_variable* priorityNewChunkMeshesCV);


		// Modifiers.

		/**
		* @brief Change the attributes used in the construction of this object.
		*/
		void setAttributes(chunk* chunk, atomicRecyclingPool<meshChunkJob>* pool, bool generateTerrain, bool isPriorityUpdate,
			std::condition_variable* priorityNewChunkMeshesCV);


	private:

		/*
		Methods.
		*/

		void process();


		/*
		Attributes.
		*/

		chunk* chunk_;
		atomicRecyclingPool<meshChunkJob>* pool_;
		bool generateTerrain_,
			 isPriorityUpdate_;
		std::condition_variable* priorityNewChunkMeshesCV_;
	
	};

	inline meshChunkJob::meshChunkJob()
	: chunk_(nullptr),
	  pool_(nullptr),
	  generateTerrain_(false),
	  isPriorityUpdate_(false),
      priorityNewChunkMeshesCV_(nullptr)
	{}

	inline meshChunkJob::meshChunkJob(chunk* chunk, atomicRecyclingPool<meshChunkJob>* pool, bool generateTerrain, bool isPriorityUpdate,
		std::condition_variable* priorityNewChunkMeshesCV)
	: chunk_(chunk),
	  pool_(pool),
	  generateTerrain_(generateTerrain),
	  isPriorityUpdate_(isPriorityUpdate),
      priorityNewChunkMeshesCV_(priorityNewChunkMeshesCV)
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
		* Update the read-only copy of the chunks' meshes that are ready to be sent to GPU.
		* Only the chunks that were modified with priority updates are updated.
		*/
		static void updatePriorityReadChunkMeshes();

		/**
		* Update the read-only copy of the chunks' meshes that are ready to be sent to GPU.
		* Only the chunks that were modified without priority updates are updated.
		*/
		static void updateReadChunkMeshes(std::unique_lock<std::mutex>& priorityUpdatesLock);

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
		static chunkLoadLevel getChunkLoadLevel(const vec3& chunkPos);

		/**
		* @brief Returns the chunk's load level.
		*/
		static chunkLoadLevel getChunkLoadLevel(int chunkX, int chunkY, int chunkZ);

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
		* @brief Select a chunk with the specified chunk position.
		* If said chunk is not created, create everything except its mesh.
		* WARNING. Not meant for use in AI mode.
		*/
		static chunk* selectChunkOrCreate(int x, int y, int z);

		/**
		* @brief Select a chunk with the specified chunk position.
		* If said chunk is not created, create everything except its mesh.
		* WARNING. Not meant for use in AI mode.
		*/
		static chunk* selectChunkOrCreate(const vec3& chunkPos);

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
		* WARNING. Non-atomic operation! Must be called when all meshing threads are synced with
		* the chunk management thread.
		* WARNING. This method is used in an infinite world.
		*/
		static int unloadFrontierChunk(const vec3& chunkPos);

		/**
		* @brief Method called by the chunk management thread to use with infinite world types.
		* It coordinates all meshing threads and manages the chunk unloading process
		* in order to prevent any race condition between said threads, among other things
		* such as synchronization and data transfering with the rendering thread.
		* This method does not handle the chunk priority updates.
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
		* @brief Load chunk at specified chunk coordinates.
		* Returns a pointer to the loaded chunk.
		* WARNING. DOES NOT CHECK IF THERE IS ALREADY A CHUNK AT THE SPECIFIED POSITION.
		*/
		static chunk* loadChunkV2(const vec3& chunkPos);

		/**
		* @brief Issue a job that will consists of rengerating the mesh of the specified chunk.
		* If 'generateTerrain' is true, it's terrain will be regenerated.
		* The job will be executed on another thread and will lock the chunk's mutexes that
		* ara required.
		*/
		static void issueChunkMeshJob(chunk* c, bool generateTerrain , bool isPriorityUpdate);

		/** 
		* @brief Used on chunkManager::onUnloadAsFrontier to update the neighbor
		* chunks of a frontier chunk that was unloaded.
		*/
		static bool onUnloadAsFrontier(chunk* chunk, double distUnloadedToPlayer);

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
		static std::list<std::list<vec3>::iterator> chunksToDefrontierize_;
		static notInRenderDistance notInRenderDistance_;
		static closestChunk closestChunk_;
		static threadPool* chunkTasks_, 
						 * priorityChunkTasks_;
		static atomicRecyclingPool<meshChunkJob>* loadChunkJobs_;
		static atomicRecyclingPool<chunk> chunksPool_;
		static chunkEvent onChunkLoad_;
		static chunkEvent onChunkUnload_;
		static vertexBuffer* vbo_;
		static std::atomic<bool> clearChunksFlag_,
			                     priorityUpdatesRemaining_;

		/**
		* @brief Called by chunkManager::addFrontier(chunk* chunk) to make the neighbors
		* frontier if necessary.
		*/
		static void updateFrontierNeighbor(chunk* frontierChunk, chunk* neighborChunk);

		// NEW THINGS END HERE

		/*
		Attributes.
		*/

		static bool initialised_;
		static int nChunksToCompute_;
		static std::unordered_map<vec3, chunk*> chunks_;
		static std::unordered_map<vec3, model>* drawableChunksRead_;
		static std::deque<chunk*> newChunkMeshes_,
								  priorityNewChunkMeshes_;
			                      
		static std::deque<vec3> chunkMeshesToDelete_; // Read-only chunk meshes that need to be deleted.
		static std::mutex managerThreadMutex_,
						  priorityManagerThreadMutex_,
						  priorityNewChunkMeshesMutex_,
						  priorityUpdatesRemainingMutex_,
						  loadingTerrainMutex_;
		static std::recursive_mutex newChunkMeshesMutex_,
								    chunkMeshesToDeleteMutex_,
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

		static unsigned int parseChunkPosState_; // 0 = parsing x coord, 1 = parsing y coord, 2 = parsing z coord.
		static const unsigned int parseChunkPosStates_;
		static std::string openedTerrainFileName_;

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

		return chunks_;

	}

	inline std::unordered_map<vec3, model> const * chunkManager::drawableChunksRead() {

		return drawableChunksRead_;

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

	inline chunkLoadLevel chunkManager::getChunkLoadLevel(int chunkX, int chunkY, int chunkZ) {

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