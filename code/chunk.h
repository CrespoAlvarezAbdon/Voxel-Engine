#ifndef _VOXELENG_CHUNK_
#define _VOXELENG_CHUNK_
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <deque>
#include <thread>
#include <atomic>
#include <mutex>
#include <shared_mutex>
#include <condition_variable>
#include <barrier>
#include <string>
#include <hash.hpp>
#include <GL/glew.h>
#include <glm.hpp>
#include "vertex_buffer.h"
#include "vertex_buffer_layout.h"
#include "shader.h"
#include "vertex.h"
#include "camera.h"
#include "texture.h"
#include "model.h"
#include "worldGen.h"
#include "definitions.h"
#include "utilities.h"


namespace VoxelEng {

	//////////////////////////
	// Forward declarations.//
	//////////////////////////

	class camera;
	class chunkManager;
	class worldGen;

	/////////////////
	//Enum classes.//
	/////////////////

	enum class chunkLoadLevel { NOTLOADED = 0, BASICTERRAIN = 1, DECORATED = 2 };


	////////////
	//Classes.//
	////////////

	/*
	Wraps up data used to render a chunk.
	*/
	struct chunkRenderingData {

		vec3 chunkPos;
		model vertices;

	};


	/*
	Represents a section of the voxel world, with its blocks, mesh, position and other infomation.
	Chunks that are marked as 'dirty' or 'changed' will have their mesh regenerated if they are withing
	a player's view range and they are visibile (W.I.P on that second part of the condition).
	*/
	class chunk {

	public:

		// Initialisers

		static void init();


		// Constructors.

		/*
		Construct an empty dirty chunk.
		If 'empty' is true then the chunk block data will be filled with null block IDs (block ID 0).
		If 'empty' is false then the chunk block data will be filled with the selected world generator.
		*/
		chunk(bool empty, const vec3& chunkPos = vec3Zero);

		chunk(const chunk& source);


		// Observers.

		/*
		Get the ID of the cube at chunk coordinates x y z.
		*/
		block getBlock(GLbyte x, GLbyte y, GLbyte z);

		block getBlock(const vec3& pos);

		/*
		Get chunk's x axis coordinate (chunk relative).
		*/
		GLbyte x() const;

		/*
		Get chunk's y axis coordinate (chunk relative).
		*/
		GLbyte y() const;

		/*
		Get chunk's z axis coordinate (chunk relative).
		*/
		GLbyte z() const;

		/*
		Get this chunk's chunk relative position.
		*/
		const vec3& chunkPos() const;

		/*
		Get this chunk's position.
		*/
		const vec3& pos() const;

		const std::vector<vertex>& vertices() const;

		const chunkRenderingData& renderingData() const;
	
		/*
		Get the number of non-null blocks (blocks with ID != 0) that exist in the chunk.
		*/
		unsigned int getNBlocks();

		const std::atomic<bool>& changed() const;

		chunkLoadLevel loadLevel() const;


		// Modifiers.

		/*
		Sets the value of a block within the chunk.
		The chunk is marked as dirty.
		Returns the old ID of the modified block.
		*/
		block setBlock(GLbyte x, GLbyte y, GLbyte z, block block_id);

		/*
		Sets the value of a block within the chunk.
		The chunk is marked as dirty.
		Returns the old ID of the modified block.
		*/
		block setBlock(const vec3& chunkRelPos, block blockID);

		/*
		Sets the value of a block within the chunk.
		The chunk is marked as dirty.
		Returns the old ID of the modified block.
		*/
		block setBlock(unsigned int linearIndex, block blockID);

		vec3& chunkPos();

		chunkRenderingData& renderingData();

		void setNBlocks(unsigned int nBlocks);

		std::shared_mutex& blockDataMutex();

		std::atomic<bool>& changed();

		/*
		Regenerate the chunk's mesh.
		*/
		void renewMesh();

		/*
		The chunk's block data will be filled with 0s, leaving the chunk "empty of blocks" or full of null blocks.
		*/
		void makeEmpty();

		void setLoadLevel(chunkLoadLevel level);


		// Clean up.
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

	

	/*
	Used for managing the chunks' life cycle, level loading...
	*/
	class chunkManager {

	public:

		// Initializers.

		static void init(unsigned int nChunksToCompute = 10);


		// Observers.

		static bool initialised();

		static vec3 getChunkRelCoords(const vec3& blockPos);

		static vec3 getChunkRelCoords(float globalX, float globalY, float globalZ);

		static vec3 getChunkCoords(const vec3& blockPos);

		static vec3 getChunkCoords(float globalX, float globalY, float globalZ);

		static vec2 getChunkXZCoords(const vec2& blockXZPos);

		static vec2 getChunkXZCoords(int blockX, int blockZ);

		static vec3 getGlobalPos(const vec3& chunkPos, const vec3& inChunkPos);

		static vec3 getGlobalPos(int chunkX, int chunkY, int chunkZ, int inChunkX, int inChunkY, int inChunkZ);

		static vec2 getXZGlobalPos(const vec2& chunkPos, const vec2& inChunkPos);

		static vec2 getXZGlobalPos(int chunkX, int chunkZ, int inChunkX, int inChunkZ);

		static const std::unordered_map<vec3, chunk*>& chunks();

		static std::unordered_map<vec3, std::vector<vertex>> const* drawableChunksRead();

		static unsigned int nChunksToCompute();

		/*
		'posX', 'posY' and 'posZ' refere to a global position.
		*/
		static block getBlock(int posX, int posY, int posZ);

		/*
		'pos' holds a global postion.
		*/
		static block getBlock(const vec3& pos);

		/*
		Get all blocks in the world that are in the box defined with the positions pos1 and pos2 taking into
		account that pos1.x <= pos2.x ^ pos1.y <= pos2.y ^ pos1.z <= pos2.z.
		*/
		static std::vector<block> getBlocksBox(const vec3& pos1, const vec3& pos2);

		static std::vector<block> getBlocksBox(int x1, int y1, int z1, int x2, int y2, int z2);

		static const std::unordered_set<vec3>& cFreeableChunks();

		static const std::atomic<bool>& cForceSyncFlag();

		/*
		When the chunk manager system is in AI mode it offers special methods such
		as the ability to create a copy of the loaded level to each AI agent in case they
		need it.
		*/
		static bool AImodeON();

		/*
		Returns if the given position is currently inside the loaded area around
		the player.
		*/
		static bool isInWorld(const vec3& pos);

		/*
		Returns if the given position is currently inside the loaded area around
		the player.
		*/
		static bool isInWorld(int x, int y, int z);

		static bool isChunkInWorld(const vec3& chunkPos);

		static bool isChunkInWorld(int chunkX, int chunkY, int chunkZ);

		static chunkLoadLevel getChunkLoadLevel(const vec3& chunkPos);

		static chunkLoadLevel getChunkLoadLevel(int chunkX, int chunkY, int chunkZ);

		static const std::string& openedTerrainFileName();

		static bool infiniteWorld();


		// Modifiers.

		/*
		Chunkmanager's AI mode does not generate the graphical part of the terrain to save
		resources when training or testing AIs (that is, when not playing a record or accessing a
		level in graphical mode).
		*/
		static void setAImode(bool ON);

		static void setNChunksToCompute(unsigned int nChunksToCompute);

		static block setBlock(const vec3& pos, block blockID);

		static block setBlock(int x, int y, int z, block blockID);

		/*
		Returns pointer to the created chunk.
		*/
		static chunk* createChunkAt(bool empty, const vec3& chunkPos);

		/*
		Returns pointer to the created chunk.
		*/
		static chunk* createChunkAt(bool empty, int chunkX, int chunkY, int chunkZ);

		/*
		Same as createChunkAt() but with no bounds checking.
		Returns pointer to the created chunk.
		*/
		static chunk* createChunk(bool empty, const vec3& chunkPos);

		/*
		Same as createChunkAt() but with no bounds checking.
		Returns pointer to the created chunk.
		*/
		static chunk* createChunk(bool empty, int chunkX, int chunkY, int chunkZ);

		static chunk* selectChunk(GLbyte x, GLbyte y, GLbyte z);

		static chunk* selectChunkByChunkPos(const vec3& chunkPos);

		static chunk* selectChunkByChunkPos(int x, int y, int z);

		/*
		Uses real float point world coordinates.
		*/
		static chunk* selectChunkByRealPos(const vec3& pos);

		static chunk* neighborMinusX(const vec3& chunkPos);

		static chunk* neighborPlusX(const vec3& chunkPos);

		static chunk* neighborMinusY(const vec3& chunkPos);

		static chunk* neighborPlusY(const vec3& chunkPos);

		static chunk* neighborMinusZ(const vec3& chunkPos);

		static chunk* neighborPlusZ(const vec3& chunkPos);

		/*
		WARNING. This operation is not thread-safe.
		To push back a chunk's rendering data into this deque,
		use chunkManager::pushDrawableChunks(...) method to prevent
		race conditions when using multiple threads in the
		chunk management system.
		*/
		static std::unordered_map<vec3, std::vector<vertex>>* drawableChunksWrite();

		static std::recursive_mutex& chunksMutex();

		static std::recursive_mutex& highPriorityListMutex();

		static std::unordered_set<vec3>& freeableChunks();

		static std::mutex& freeableChunksMutex();

		static std::mutex& managerThreadMutex();

		static std::condition_variable& managerThreadCV();

		static std::atomic<bool>& forceSyncFlag();

		/*
		Locks the calling thread until:
		- Minimal terrain has been loaded (infinite world).
		- The entire level has been loaded (finite world).
		*/
		static void waitTerrainLoaded();

		/*
		Atomically pushes back a chunk's rendering data into the write drawable chunks deque.
		*/
		static void pushDrawableChunks(const chunkRenderingData& renderingData);

		/*
		WARNING. ONLY CALL THIS METHOD WHEN THE RENDERER THREAD AND THE CHUNK MANAGER THREAD ARE SYNCED.
		Swaps the buffers of chunks' vertices. The one used for reading in the renderer thread becomes
		the one used for writing in the chunk manager thread and vice versa.
		*/
		static void swapDrawableChunksLists();

		/*
		WARNING. ONLY CALL THIS METHOD WHEN THE RENDERER THREAD AND THE CHUNK MANAGER THREAD ARE SYNCED.
		Update vertex data of all chunks that have a pending hign priority update.
		*/
		static void updatePriorityChunks();

		/*
		WARNING. This method is used in with an infinite world.
		Atomically loads a new chunk at the specified 'chunkPos' chunk position,
		overwriting any chunks that were already at that position, if any.
		*/
		static void loadChunk(const vec3& chunkPos);

		/*
		WARNING. Non-atomic operation! Must be called when all meshing threads are synced with
		the chunk management thread.
		Unloads the chunk at chunk position 'chunkPos', pushing it into a free chunks deque
		to be reused later for another chunk position of the world.
		*/
		static void unloadChunk(const vec3& chunkPos);

		/*
		Function called by meshing threads to generate
		meshes for chunks that are close to the player.
		*/
		static void meshChunks(const std::atomic<int>& chunkRange, int rangeStart, int rangeEnd,
						       std::shared_mutex& syncMutex, std::condition_variable_any& meshingThreadsCV,
						       std::atomic<bool>& meshingTsCVFlag, std::barrier<>& syncPoint);

		/*
		Function called by the chunk management thread.
		It coordinates all meshing threads and manages the chunk unloading process
		in order to prevent any race condition between said threads, among other things
		such as synchronization and data transfering with the rendering thread.
		*/
		static void manageChunks(unsigned int nMeshingThreads);

		/*
		Legacy finite world loading.
		Playing AI match records is only supported by this world loading type.
		If 'terrainFile' is equal to "", then either a new level will be generated or
        a level from a slot will be loaded (depending on the selected save slot).
		*/
		static void finiteWorldLoading(const std::string& terrainFile = "");

		/*
		Generates only block data without the 3D graphics side.
		Intended to be used when testing or training AI agents
		without displaying the matches in the engine's graphical mode
		to save CPU and GPU processing power.
		This generated world is the original copy. Copies for each
		AI agent will be generated in chunks as they are needed by
		their respective agents.
		If 'path' is equal to "" then a randomly generated world will
		be created. Otherwise it will load de .terrain file
		located at 'path' + ".terrain".
		*/
		static void generateAIWorld(const std::string& path = "");

		/*
		Saves all loaded chunks.
		This is intended to be used along with finite world loading.
		The .terrain file extension is automatically appended.
		*/
		static void saveAllChunks(const std::string& path);

		/*
		Loads all chunks from the specified TERRAIN file.
		This is intended to be used along with finite world loading.
		The .terrain file extension is automatically appended.
		*/
		static void loadAllChunks(const std::string& path);

		/*
		Queue a chunk in the high priority update list.
		Only chunks with high priority for being remeshed
		(such as chunks modified by the player) should be
		queued into this list.
		*/
		static void highPriorityUpdate(const vec3& chunkPos);

		/*
		Get block and set block operations in the chunk manager system will now
		be performed on the AI world/level of AI agent with ID 'individualID'.
		AI mode must be turned on in the chunk manager system.
		WARNING. This method is not thread safe.
		*/
		static void selectAIworld(unsigned int individualID);

		/*
		ONLY get block operations in the chunk manager system will now
		be performed on the original copy of the level that is being used for the AI game.
		AI mode must be turned on in the chunk manager system.
		Set block operations will use the latest AI world selected (the one corresponding to
		the AI agent with ID 0 by default).
		WARNING. This method is not thread safe.
		*/
		static void selectOriginalWorld();


		// Clean Up.

		/*
		Frees any memory allocated in the process of generating the world, like
		all the Chunk objects created to load it.
		*/
		static void cleanUp();

	private:

		static bool initialised_,
					infiniteWorld_;
		static int nChunksToCompute_;
		static std::unordered_map<vec3, chunk*> chunks_;
		static std::unordered_map<vec3, std::vector<vertex>>* drawableChunksWrite_,
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

		static std::unordered_map<unsigned int, std::unordered_map<vec3, chunk*>> AIagentChunks_;
		static unsigned int selectedAIWorld_;

		static bool AImodeON_,
			        originalWorldAccess_;

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

	inline bool chunkManager::AImodeON() {
	
		return AImodeON_;
	
	}

	inline bool chunkManager::isInWorld(const vec3& pos) {
	
		return isInWorld(pos.x, pos.y, pos.z);
	
	}

	inline bool chunkManager::isInWorld(int x, int y, int z) {

		return x >= -nChunksToCompute_ * SCX && x < (nChunksToCompute_ - 1) * SCX &&
			   y >= yChunksRange * SCY && y < (yChunksRange - 1) * SCY &&
			   z >= -nChunksToCompute_ * SCZ && x < (nChunksToCompute_ - 1) * SCZ;
	
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