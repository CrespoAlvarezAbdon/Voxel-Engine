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
#include <deque>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <thread>
#include <mutex>
#include <shared_mutex>
#include <string>
#include <hash.hpp>

#if GRAPHICS_API == OPENGL

#include <GL/glew.h>
#include <glm.hpp>

#endif

#include "vertexBuffer.h"
#include "vertexBufferLayout.h"
#include "shader.h"
#include "vertex.h"
#include "camera.h"
#include "texture.h"
#include "model.h"
#include "worldGen.h"
#include "definitions.h"
#include "utilities.h"


namespace VoxelEng {

	/////////////////////////
	//Forward declarations.//
	/////////////////////////

	class camera;
	class chunkManager;
	class worldGen;


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
		model vertices;

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
		or false otherwise.
		*/
		static bool initialised();

		/**
		* @brief Get the ID of the cube at the specifeid chunk-local coordinates.
		*/
		block getBlock(GLbyte x, GLbyte y, GLbyte z);

		/**
		* @brief Get the ID of the cube at the specifeid chunk-local coordinates.
		*/
		block getBlock(const vec3& inChunkPos);

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
		* @brief Get this chunk's vertex data.
		*/
		const std::vector<vertex>& vertices() const;

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
		const std::atomic<bool>& changed() const;

		/**
		* @brief Returns the chunk's load level.
		*/
		chunkLoadLevel loadLevel() const;


		// Modifiers.

		/**
		* @brief Sets the value of a block within the chunk.
		* The chunk is marked as dirty.
		* Returns the old ID of the modified block.
		*/
		block setBlock(GLbyte x, GLbyte y, GLbyte z, block block_id);

		/**
		* @brief Sets the value of a block within the chunk.
		* The chunk is marked as dirty.
		* Returns the old ID of the modified block.
		*/
		block setBlock(const vec3& chunkRelPos, block blockID);

		/**
		* @brief Sets the value of a block within the chunk.
		* The chunk is marked as dirty.
		* Returns the old ID of the modified block.
		*/
		block setBlock(unsigned int linearIndex, block blockID);

		/**
		* @brief Returns the chunk's chunk position.
		*/
		vec3& chunkPos();

		/**
		* @brief Returns the chunk's rendering data object.
		*/
		chunkRenderingData& renderingData();

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
		* @brief Regenerate the chunk's mesh.
		*/
		void renewMesh();

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


		/*
		Clean up.
		*/

		/**
		* @brief Clean up any resources allocated for the class' static methods.
		*/ 
		static void cleanUp();
	
	private:

		static bool initialised_;
		static const model* blockVertices_;
		static const modelTriangles* blockTriangles_;

		block blocks_[SCX][SCY][SCZ];
		std::atomic<bool> changed_;
		std::atomic<unsigned int> nBlocks_;
		std::atomic<chunkLoadLevel> loadLevel_;
		chunkRenderingData renderingData_;

		/*
		Used for reading the block data in a chunk.
		All meshing threads only read this data, so they access it
		in shared mode, while any thread that can alter this data
		must access it in exclusive/unique mode.
		*/
		std::shared_mutex blocksMutex_;

	};

	inline bool chunk::initialised() {
	
		return initialised_;
	
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

		return vec3(renderingData_.chunkPos.x * SCX, renderingData_.chunkPos.y * SCY, renderingData_.chunkPos.z * SCZ);

	}

	inline const std::vector<vertex>& chunk::vertices() const {

		return renderingData_.vertices;

	}

	inline const chunkRenderingData& chunk::renderingData() const {

		return renderingData_;

	}

	inline unsigned int chunk::getNBlocks() {

		return nBlocks_;

	}

	inline const std::atomic<bool>& chunk::changed() const {

		return changed_;

	}

	inline chunkLoadLevel chunk::loadLevel() const {

		return loadLevel_;

	}

	inline chunkRenderingData& chunk::renderingData() {

		return renderingData_;

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

	

	/**
	* @brief Used for managing the chunks' life cycle, level loading...
	*/
	class chunkManager {

	public:

		// Initializers.

		/**
		* @brief Initialise the chunk management system.
		*/
		static void init(unsigned int nChunksToCompute = DEF_N_CHUNKS_TO_COMPUTE);


		// Observers.

		/**
		* @brief Returns true if the system is initialised or false otherwise.
		*/
		static bool initialised();

		/**
		* @brief Returns chunk local coordinates of a global position.
		*/
		static vec3 getChunkRelCoords(const vec3& blockPos);

		/**
		* @brief Returns chunk local coordinates of a global position.
		*/
		static vec3 getChunkRelCoords(float globalX, float globalY, float globalZ);

		/**
		* @brief Returns chunk grid coordinates of a global position.
		*/
		static vec3 getChunkCoords(const vec3& blockPos);

		/**
		* @brief Returns chunk local coordinates of a global position.
		*/
		static vec3 getChunkCoords(float globalX, float globalY, float globalZ);

		/**
		* @brief Returns chunk coordinates in the X and Z axes of a global position in the X and Z axes.
		*/
		static vec2 getChunkXZCoords(const vec2& blockXZPos);

		/**
		* @brief Returns chunk coordinates in the X and Z axes of a global position in the X and Z axes.
		*/
		static vec2 getChunkXZCoords(int blockX, int blockZ);

		/**
		* @brief Returns global coordinates of a local chunk coordinate from a certain chunk position.
		*/
		static vec3 getGlobalPos(const vec3& chunkPos, const vec3& inChunkPos);

		/**
		* @brief Returns global coordinates of a local chunk coordinate from a certain chunk position.
		*/
		static vec3 getGlobalPos(int chunkX, int chunkY, int chunkZ, int inChunkX, int inChunkY, int inChunkZ);

		/**
		* @brief Returns global coordinates in the X and Z axes of a local chunk coordinate from a certain chunk position,
		* both in the X and Z axes.
		*/
		static vec2 getXZGlobalPos(const vec2& chunkPos, const vec2& inChunkPos);

		/**
		* @brief Returns global coordinates in the X and Z axes of a local chunk coordinate from a certain chunk position,
		* both in the X and Z axes.
		*/
		static vec2 getXZGlobalPos(int chunkX, int chunkZ, int inChunkX, int inChunkZ);

		/**
		* @brief Returns the system's dictionary of registered chunks.
		*/
		static const std::unordered_map<vec3, chunk*>& chunks();

		/**
		* @brief Returns the system's readable chunk vertex data. That is, it returns the system's chunk
		* vertex data that is safe to read for the rendering thread in order to render them.
		*/
		static std::unordered_map<vec3, std::vector<vertex>> const* drawableChunksRead();

		static unsigned int nChunksToCompute();

		/**
		* @brief Get the block ID of a specified global position.
		*/
		static block getBlock(int posX, int posY, int posZ);

		/**
		* @brief Get the block ID of a specified global position.
		*/
		static block getBlock(const vec3& pos);

		/**
		* @brief Get all blocks in the world that are in the box defined with the positions pos1 and pos2
		*/
		static std::vector<block> getBlocksBox(const vec3& pos1, const vec3& pos2);

		/**
		* @brief Get all blocks in the world that are in the box defined with the positions pos1 and pos2
		*/
		static std::vector<block> getBlocksBox(int x1, int y1, int z1, int x2, int y2, int z2);

		/**
		* @brief Returns the chunk position of the system's freeable chunks.
		*/
		static const std::unordered_set<vec3>& cFreeableChunks();

		/**
		* @brief Returns the flag that is used to force synchronisation between the chunk management
		* and the rendering thread.
		*/
		static const std::atomic<bool>& cForceSyncFlag();

		/**
		* @brief Returns true if the given block position is currently inside the loaded area around
		* the player or false otherwise.
		*/
		static bool isInWorld(const vec3& pos);

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
		* @brief Returns true if the system is currently using the infinite world type
		* or false otherwise.
		*/
		static bool infiniteWorld();


		// Modifiers.

		/**
		* @brief Turns the system's AI mode on or off.
		*/
		static void setAImode(bool on);

		/**
		* @brief Set the number of chunks to compute in the X and Z axes.
		*/
		static void setNChunksToCompute(unsigned int nChunksToCompute);

		/**
		* @brief Set the block ID of a specfied block position.
		*/
		static block setBlock(const vec3& pos, block blockID);

		/**
		* @brief Set the block ID of a specfied block position.
		*/
		static block setBlock(int x, int y, int z, block blockID);

		/**
		* @brief Returns pointer to the created chunk.
		*/
		static chunk* createChunkAt(bool empty, const vec3& chunkPos);

		/**
		* @brief Returns pointer to the created chunk.
		*/
		static chunk* createChunkAt(bool empty, int chunkX, int chunkY, int chunkZ);

		/**
		* @brief Same as chunkManager::createChunkAt() but with no bounds checking.
		* Returns pointer to the created chunk.
		*/
		static chunk* createChunk(bool empty, const vec3& chunkPos);

		/**
		* @brief Same as chunkManager::createChunkAt() but with no bounds checking.
		* Returns pointer to the created chunk.
		*/
		static chunk* createChunk(bool empty, int chunkX, int chunkY, int chunkZ);

		/**
		* @brief Select a chunk with the specified chunk position.
		* WARNING. Not meant for use in AI mode.
		*/
		static chunk* selectChunk(int x, int y, int z);

		/**
		* @brief Select a chunk with the specified chunk position.
		* WARNING. Not meant for use in AI mode.
		*/
		static chunk* selectChunkByChunkPos(const vec3& chunkPos);

		/**
		* @brief Select a chunk with the specified block position.
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
		* @brief Returns the system's writeable chunk vertex data. That is,
		* it returns the system's chunk vertex data that can be modified by the chunk
		* management thread and chunk meshing threads and that it is not safe for
		* reading.
		* WARNING. This operation is not thread-safe.
		* To push back a chunk's rendering data into this deque,
		* use chunkManager::pushDrawableChunks(...) method to prevent
		* race conditions when using multiple threads in the
		* chunk management system.
		*/
		static std::unordered_map<vec3, std::vector<vertex>>* drawableChunksWrite();

		/**
		* @brief Returns the mutex that guards the registered chunks dictionary.
		* WARNING. Not meant for use in AI mode.
		*/
		static std::recursive_mutex& chunksMutex();

		/**
		* @brief Returns the mutex that guards the chunk high priority update list.
		* WARNING. Not meant for use in AI mode.
		*/
		static std::recursive_mutex& highPriorityListMutex();

		/**
		* @brief Returns the list of freeable chunks.
		* WARNING. Not meant for use in AI mode.
		*/
		static std::unordered_set<vec3>& freeableChunks();

		/**
		* @brief Returns the mutex that guards the freeable chunk list.
		* WARNING. Not meant for use in AI mode.
		*/
		static std::mutex& freeableChunksMutex();

		/**
		* @brief Returns the mutex associated with the chunk management thread.
		* WARNING. Not meant for use in AI mode.
		*/
		static std::mutex& managerThreadMutex();

		/**
		* @brief Returns the condition variable associated with the chunk management thread.
		* WARNING. Not meant for use in AI mode.
		*/
		static std::condition_variable& managerThreadCV();
		
		/**
		* @brief Returns the flag used to force synchronisation the rendering
		* and chunk management threads.
		* WARNING. Not meant for use in AI mode.
		*/
		static std::atomic<bool>& forceSyncFlag();

		/**
		* @brief Locks the calling thread until:
		* - Minimal terrain has been loaded around the player (infinite world).
		* - The entire level has been loaded (finite world).
		*/
		static void waitTerrainLoaded();

		/**
		* @brief Atomically pushes back a chunk's rendering data into the write drawable chunks deque.
		*/
		static void pushDrawableChunks(const chunkRenderingData& renderingData);

		/**
		* @brief Swaps the buffers of chunks' vertices. The one used for reading in the renderer thread becomes
		* the one used for writing in the chunk manager thread and vice versa.
		* Sets the meshes updated flag to false and should be called only if it is set to true (check
		* with chunkManager::meshesUpdated().
		* WARNING. ONLY CALL THIS METHOD WHEN THE RENDERER THREAD AND THE CHUNK MANAGER THREAD ARE SYNCED.
		*/
		static void swapDrawableChunksLists();

		/**
		* @brief Update vertex data of all chunks that have a pending hign priority update.
		* WARNING. ONLY CALL THIS METHOD WHEN THE RENDERER THREAD AND THE CHUNK MANAGER THREAD ARE SYNCED.
		*/
		static void updatePriorityChunks();

		/**
		* @brief Atomically loads a new chunk at the specified 'chunkPos' chunk position,
		* overwriting any chunks that were already at that position, if any.
		* WARNING. This method is used in with an infinite world.
		*/
		static void loadChunk(const vec3& chunkPos);

		/**
		* @brief Unloads the chunk at chunk position 'chunkPos', pushing it into a free chunks deque
		* to be reused later for another chunk position of the world.
		* WARNING. Non-atomic operation! Must be called when all meshing threads are synced with
		* the chunk management thread.
		* WARNING. This method is used in with an infinite world.
		*/
		static void unloadChunk(const vec3& chunkPos);

		/**
		* @brief Function called by meshing threads to generate
		* meshes for chunks that are close to the player.
		*/
		static void meshChunks(const std::atomic<int>& chunkRange, int rangeStart, int rangeEnd,
						       std::shared_mutex& syncMutex, std::condition_variable_any& meshingThreadsCV,
						       std::atomic<bool>& meshingTsCVFlag, std::barrier<>& syncPoint);

		/**
		* @brief Function called by the chunk management thread to use with infinite world types.
		* It coordinates all meshing threads and manages the chunk unloading process
		* in order to prevent any race condition between said threads, among other things
		* such as synchronization and data transfering with the rendering thread.
		* WARNING. CURRENTLY NOT TESTED, ONLY PROOF OF CONCEPT.
		*/
		static void manageChunks(unsigned int nMeshingThreads);

		/**
		* @brief Finite world loading function for the chunk management thread.
		* Playing AI match records is only supported by this world loading type.
		* If 'terrainFile' is equal to "", then either a new level will be generated or
        * a level from a slot will be loaded (depending on the selected save slot).
		*/
		static void finiteWorldLoading(const std::string& terrainFile = "");

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
		* @brief Saves all loaded chunks.
		* This is intended to be used along with finite world loading.
		* The .terrain file extension is automatically appended.
		*/
		static void saveAllChunks(const std::string& path);

		/**
		* @brief Loads all chunks from the specified TERRAIN file.
		* This is intended to be used along with finite world loading.
		* The .terrain file extension is automatically appended.
		*/
		static void loadAllChunks(const std::string& path);

		/**
		* @brief Queue a chunk in the high priority update list.
		* Only chunks with high priority for being remeshed
		* (such as chunks modified by the player) should be
		* queued into this list.
		*/
		static void highPriorityUpdate(const vec3& chunkPos);

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


		/*
		Clean Up.
		*/

		/**
		* @brief Only cleans any resources like chunks created but it does not de-initialise
		* the chunk management system.
		*/
		static void clean();

		/**
		* @brief Frees any memory allocated in the process of generating the world, like
		* all the Chunk objects created to load it.
		*/
		static void cleanUp();

	private:

		/*
		Attributes.
		*/

		static bool initialised_,
					infiniteWorld_;
		static int nChunksToCompute_;
		static std::unordered_map<vec3, chunk*> chunks_;
		static std::unordered_map<vec3, model>* drawableChunksWrite_,
															* drawableChunksRead_;
		static std::deque<chunk*> freeChunks_;
		static std::unordered_set<vec3> freeableChunks_;
	
		static std::deque<vec3> priorityMeshingList_; // Chunks that need a high priority mesh regeneration.
		static std::deque<vec3> priorityUpdateList_; // Chunks that, once their mesh is updated, need to update their vertex data.

		static std::mutex freeableChunksMutex_,
			              managerThreadMutex_,
						  loadingTerrainMutex_;

		/*
		This mutex's sole purpose is to let us
		use the highPriorityUpdatesCV_ condition
		variable.
		*/
		static std::shared_mutex highPriorityMutex_;

		static std::recursive_mutex drawableChunksWriteMutex_,
						            chunksMutex_,
						            freeChunksMutex_,
						            priorityMeshingListMutex_;
		static std::condition_variable managerThreadCV_,
									   loadingTerrainCV_;
		static std::condition_variable_any highPriorityUpdatesCV_;

		/*
		Used to force all meshing threads to synchronize with
		the rendering thread when a high priority
		chunk update is issued and it's imperative than the update
		made is reflected in the rendering thread as quickly as possible.
		*/
		static std::atomic<bool> forceSyncFlag_,
							     waitTerrainLoaded_;

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
		static block getBlockOGWorld_(int posX, int posY, int posZ);

	};

	inline bool chunkManager::initialised() {
	
		return initialised_;
	
	}

	inline vec3 chunkManager::getChunkRelCoords(const vec3& blockPos) {
	
		return getChunkRelCoords(blockPos.x, blockPos.y, blockPos.z);
	
	}

	inline vec3 chunkManager::getChunkCoords(const vec3& blockPos) {
	
		return getChunkCoords(blockPos.x, blockPos.y, blockPos.z);
	
	}

	inline vec3 chunkManager::getChunkCoords(float globalX, float globalY, float globalZ) {

		return vec3(floor(globalX / SCX), floor(globalY / SCY), floor(globalZ / SCZ));

	}

	inline vec2 chunkManager::getChunkXZCoords(const vec2& blockXZPos) {
	
		return getChunkXZCoords(blockXZPos.x, blockXZPos.y);
	
	}

	inline vec2 chunkManager::getChunkXZCoords(int x, int z) {

		return vec2((int)floor((double)x / SCX), (int)floor((double)z / SCY));

	}

	inline vec3 chunkManager::getGlobalPos(const vec3& chunkPos, const vec3& inChunkPos) {
	
		return getGlobalPos(chunkPos.x, chunkPos.y, chunkPos.z, inChunkPos.x, inChunkPos.y, inChunkPos.z);
	
	}
	
	inline vec3 chunkManager::getGlobalPos(int chunkX, int chunkY, int chunkZ, int inChunkX, int inChunkY, int inChunkZ) {

		return vec3(chunkX * SCX + inChunkX, chunkY * SCY + inChunkY, chunkZ * SCZ + inChunkZ);

	}

	inline vec2 chunkManager::getXZGlobalPos(const vec2& chunkPos, const vec2& inChunkPos) {

		return getXZGlobalPos(chunkPos.x, chunkPos.y, inChunkPos.x, inChunkPos.y);

	}

	inline vec2 chunkManager::getXZGlobalPos(int chunkX, int chunkZ, int inChunkX, int inChunkZ) {

		return vec2(chunkX * SCX + inChunkX, chunkZ * SCZ + inChunkZ);

	}

	inline const std::unordered_map<vec3, chunk*>& chunkManager::chunks() {

		return chunks_;

	}

	inline std::unordered_map<vec3, std::vector<vertex>> const * chunkManager::drawableChunksRead() {

		return drawableChunksRead_;

	}

	inline std::unordered_map<vec3, std::vector<vertex>>* chunkManager::drawableChunksWrite() {

		return drawableChunksWrite_;

	}

	inline unsigned int chunkManager::nChunksToCompute() {

		return nChunksToCompute_;

	}

	inline block chunkManager::getBlock(const vec3& pos) {

		return getBlock(pos.x, pos.y, pos.z);

	}

	inline std::vector<block> chunkManager::getBlocksBox(const vec3& pos1, const vec3& pos2) {

		// std::vector has move semantics. This allows us to avoid the unnecessary copies that would otherwise be made here.
		return getBlocksBox(pos1.x, pos1.y, pos1.z, pos2.x, pos2.y, pos2.z); 

	}

	inline const std::unordered_set<vec3>& chunkManager::cFreeableChunks(){

		return freeableChunks_;

	}

	inline const std::atomic<bool>& chunkManager::cForceSyncFlag() {

		return forceSyncFlag_;

	}

	inline bool chunkManager::isInWorld(const vec3& pos) {
	
		return isInWorld(pos.x, pos.y, pos.z);
	
	}

	inline bool chunkManager::isInWorld(int x, int y, int z) {

		return x >= -nChunksToCompute_ * VoxelEng::SCX && x < (nChunksToCompute_ - 1) * VoxelEng::SCX &&
			   y >= -yChunksRange * VoxelEng::SCY && y < (yChunksRange - 1) * VoxelEng::SCY &&
			   z >= -nChunksToCompute_ * VoxelEng::SCZ && z < (nChunksToCompute_ - 1) * VoxelEng::SCZ;
	
	}

	inline bool chunkManager::isChunkInWorld(const vec3& chunkPos) {

		return isChunkInWorld(chunkPos.x, chunkPos.y, chunkPos.z);

	}

	inline bool chunkManager::isChunkInWorld(int chunkX, int chunkY, int chunkZ) {

		return  chunkX >= -nChunksToCompute_ && chunkX < nChunksToCompute_&&
				chunkY >= -yChunksRange && chunkY < yChunksRange &&
				chunkZ >= -nChunksToCompute_ && chunkZ < nChunksToCompute_;

	}

	inline chunkLoadLevel chunkManager::getChunkLoadLevel(int chunkX, int chunkY, int chunkZ) {

		return getChunkLoadLevel({ chunkX, chunkY, chunkZ });

	}

	inline const std::string& chunkManager::openedTerrainFileName() {

		return openedTerrainFileName_;

	}

	inline bool chunkManager::infiniteWorld() {
	
		return infiniteWorld_;
	
	}
	
	inline block chunkManager::setBlock(const vec3& pos, block blockID) {

		return setBlock(pos.x, pos.y, pos.z, blockID);

	}

	inline chunk* chunkManager::createChunkAt(bool empty, int chunkX, int chunkY, int chunkZ) {

		return createChunkAt(empty, vec3(chunkX, chunkY, chunkZ));

	}

	inline chunk* chunkManager::createChunk(bool empty, int chunkX, int chunkY, int chunkZ) {

		return createChunk(empty, vec3(chunkX, chunkY, chunkZ));

	}

	inline std::recursive_mutex& chunkManager::chunksMutex() {

		return chunksMutex_;

	}

	inline std::recursive_mutex& chunkManager::highPriorityListMutex() {

		return priorityMeshingListMutex_;

	}

	inline std::unordered_set<vec3>& chunkManager::freeableChunks() {

		return freeableChunks_;

	}

	inline std::mutex& chunkManager::freeableChunksMutex() {

		return freeableChunksMutex_;

	}

	inline std::mutex& chunkManager::managerThreadMutex() {

		return managerThreadMutex_;

	}

	inline std::condition_variable& chunkManager::managerThreadCV() {

		return managerThreadCV_;

	}

	inline std::atomic<bool>& chunkManager::forceSyncFlag() {

		return forceSyncFlag_;

	}

}

#endif