#ifndef _CHUNK_
#define _CHUNK_

#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <deque>
#include <thread>
#include <atomic>
#include <mutex>
#include <shared_mutex>
#include <barrier>
#include <GL/glew.h>
#include <glm.hpp>
#include <hash.hpp>
#include "vertex_buffer.h"
#include "vertex_buffer_layout.h"
#include "shader.h"
#include "vertex.h"
#include "camera.h"
#include "texture.h"
#include "definitions.h"
#include "model.h"
using namespace std;


//////////////////////////////
//Forward class declarations//
//////////////////////////////

class camera;
class chunkManager;

///////////
//Structs//
///////////

/*
Wraps up data used to render a chunk.
*/
struct chunkRenderingData
{

	glm::vec3 chunkPos;
	VoxelEng::model vertices;

};

/*
Position inside a chunk.
*/
typedef VoxelEng::vec3<VoxelEng::byte> chunkRelativePos;

/* 
A chunk's position in the world.
Two chunks in the same world cannot have the same
chunkPos.
*/
typedef VoxelEng::vec3<int> chunkPos;


///////////
//Classes//
///////////


/*
Represents a section of the voxel world, with its blocks, mesh, position and other infomation.
Chunks that are marked as 'dirty' or 'changed' will have their mesh regenerated if they are withing
a player's view range and they are visibile (still W.I.P on that second part of the condition).
*/
class chunk
{

public:

	// Constructors.

	/*
	Construct an empty dirty chunk.
	*/
	chunk(chunkManager& chunkManager);


	// Observers.

	/*
	Get the ID of the cube at chunk coordinates x y z.
	*/
	VoxelEng::block getBlock(GLbyte x, GLbyte y, GLbyte z);

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
	const glm::vec3& chunkPos() const;

	/*
	Get this chunk's position.
	*/
	const glm::vec3& pos() const;

	const vector<vertex>& vertices() const;

	const chunkRenderingData& renderingData() const;
	
	unsigned int getNBlocks();

	const std::atomic<bool>& changed() const;


	// Modifiers.

	/*
	Sets the value of a block within the chunk.
	The chunk is marked as dirty.
	*/
	void setBlock(GLbyte x, GLbyte y, GLbyte z, VoxelEng::block block_id);

	/*
	Sets the value of a block within the chunk.
	The chunk is marked as dirty.
	*/
	void setBlock(chunkRelativePos chunkRelPos, VoxelEng::block blockID);

	glm::vec3& chunkPos();

	chunkRenderingData& renderingData();

	void setNBlocks(unsigned int nBlocks);

	shared_mutex& blockDataMutex();

	std::atomic<bool>& changed();


	// Other methods.
	/*
	Regenerate the chunk's mesh.
	*/
	void renewMesh();
	
private:

	VoxelEng::block blocks_[16][16][16];
	atomic<bool> changed_;
	atomic<unsigned int> nBlocks_;
	chunkRenderingData renderingData_;
	chunkManager& chunkManager_;

	/*
	Used for reading the block data in a chunk.
	All meshing threads only read this data, so they access it
	in shared mode, while any thread that can alter this data
	must access it in exclusive/unique mode.
	*/
	shared_mutex blocksMutex_;

};

inline VoxelEng::block chunk::getBlock(GLbyte x, GLbyte y, GLbyte z)
{

	return blocks_[x][y][z];

}

inline GLbyte chunk::x() const
{

	return renderingData_.chunkPos.x;

}

inline GLbyte chunk::y() const
{

	return renderingData_.chunkPos.y;

}

inline GLbyte chunk::z() const
{

	return renderingData_.chunkPos.z;

}

inline const glm::vec3& chunk::chunkPos() const 
{

	return renderingData_.chunkPos;

}

inline glm::vec3& chunk::chunkPos()
{

	return renderingData_.chunkPos;
	
}

inline const glm::vec3& chunk::pos() const
{

	return glm::vec3(renderingData_.chunkPos.x * SCX, renderingData_.chunkPos.y * SCY, renderingData_.chunkPos.z * SCZ);

}

inline const vector<vertex>& chunk::vertices() const
{

	return renderingData_.vertices;

}

inline const chunkRenderingData& chunk::renderingData() const {

	return renderingData_;

}

inline const std::atomic<bool>& chunk::changed() const
{

	return changed_;

}

inline chunkRenderingData& chunk::renderingData() {

	return renderingData_;

}

inline shared_mutex& chunk::blockDataMutex()
{

	return blocksMutex_;

}

inline std::atomic<bool>& chunk::changed()
{

	return changed_;

}

/*
Used for managing the chunk life cycle.
A chunk manager object has two deques of drawable chunks,
one for the rendering thread which is read only and another write only
which the rendering thread should NEVER access.
*/
class chunkManager
{

public:

	// Constructors.

	chunkManager(int nChunksToDraw, const camera& playerCamera);


	// Observers.

	const unordered_map<glm::vec3, chunk*>& chunks();

	unordered_map<glm::vec3, vector<vertex>> const* drawableChunksRead() const;

	int nChunksToDraw() const;

	VoxelEng::block getBlock(const glm::vec3& pos);

	const unordered_set<glm::vec3>& freeableChunks() const;

	const mutex& freeableChunksMutex() const;

	const mutex& managerThreadMutex() const;

	const condition_variable& managerThreadCV() const;

	const atomic<bool>& forceSyncFlag() const;


	// Modifiers.

	chunk* selectChunk(GLbyte x, GLbyte y, GLbyte z);

	chunk* selectChunkByChunkPos(const glm::vec3& chunkPos);

	/*
	Uses real float point coordinates.
	*/
	chunk* selectChunkByRealPos(const glm::vec3& pos);

	chunk* neighborMinusX(const glm::vec3& chunkPos);

	chunk* neighborPlusX(const glm::vec3& chunkPos);

	chunk* neighborMinusY(const glm::vec3& chunkPos);

	chunk* neighborPlusY(const glm::vec3& chunkPos);

	chunk* neighborMinusZ(const glm::vec3& chunkPos);

	chunk* neighborPlusZ(const glm::vec3& chunkPos);

	/*
	WARNING. This operation is not thread-safe.
	To push back a chunk's rendering data into this deque,
	use chunkManager::pushDrawableChunks(...) method to prevent
	race conditions when using multiple threads in the
	chunk management system.
	*/
	unordered_map<glm::vec3, vector<vertex>>* drawableChunksWrite();

	recursive_mutex& chunksMutex();

	recursive_mutex& highPriorityListMutex();

	unordered_set<glm::vec3>& freeableChunks();

	mutex& freeableChunksMutex();

	mutex& managerThreadMutex();

	condition_variable& managerThreadCV();

	atomic<bool>& forceSyncFlag();


	// Other methods.

	/*
	Atomically pushes back a chunk's rendering data into the write drawable chunks deque.
	*/
	void pushDrawableChunks(const chunkRenderingData& renderingData);

	/*
	WARNING. ONLY CALL THIS METHOD WHEN THE RENDERER THREAD AND THE CHUNK MANAGER THREAD ARE SYNCED.
	Swaps the buffers of chunks' vertices. The one used for reading in the renderer thread becomes
	the one used for writing in the chunk manager thread and vice versa.
	*/
	void swapDrawableChunksLists();

	/*
	WARNING. ONLY CALL THIS METHOD WHEN THE RENDERER THREAD AND THE CHUNK MANAGER THREAD ARE SYNCED.
	Update vertex data of all chunks that have a pending hign priority update.
	*/
	void updatePriorityChunks();

	/*
	Atomically loads a new chunk at the specified 'chunkPos' chunk position,
	overwriting any chunks that were already at that position, if any.
	*/
	void loadChunk(const glm::vec3& chunkPos);

	/*
	WARNING. Non-atomic operation! Must be called when all meshing threads are synced with
	the chunk management thread.
	Unloads the chunk at chunk position 'chunkPos', pushing it into a free chunks deque
	to be reused later for another chunk position of the world.
	*/
	void unloadChunk(const glm::vec3& chunkPos);

	/*
	Function called by meshing threads to generate
	meshes for chunks that are close to the player.
	*/
	void meshChunks(const atomic<bool>& appFinished, const atomic<int>& chunkRange,
				    int rangeStart, int rangeEnd, shared_mutex& syncMutex, condition_variable_any& meshingThreadsCV,
					atomic<bool>& meshingTsCVFlag, barrier<>& syncPoint);

	/*
	Function called by the chunk management thread.
	It coordinates all meshing threads and manages the chunk unloading process
	in order to prevent any race condition between said threads, among other things
	such as synchronization and data transfering with the rendering thread.
	*/
	void manageChunks(const atomic<bool>& appFinished, unsigned int nMeshingThreads);

	/*
	Queue a chunk in the high priority update list.
	Only chunks with high priority for being remeshed
	(such as chunks modified by the player) should be
	queued into this list.
	*/
	void highPriorityUpdate(const glm::vec3& chunkPos);


	// Destructors.

	/*
	Frees any memory allocated in the process of generating the world, like
	all the Chunk objects created to load it.
	*/
	~chunkManager();

private:

	const camera& playerCamera_;
	int nChunksToDraw_;
	unordered_map<glm::vec3, chunk*> chunks_;
	unordered_map<glm::vec3, vector<vertex>>* drawableChunksWrite_,
										    * drawableChunksRead_;
	deque<chunk*> freeChunks_;
	unordered_set<glm::vec3> freeableChunks_;
	
	deque<glm::vec3> priorityMeshingList_; // Chunks that need a high priority mesh regeneration.
	deque<glm::vec3> priorityUpdateList_; // Chunks that, once their mesh is updated, need to update their vertex data.

	// TODO. Convert all mutex into recursive_mutex.
	mutex freeableChunksMutex_,
		  managerThreadMutex_;

	/*
	This mutex's sole purpose is to let us
	use the highPriorityUpdatesCV_ condition
	variable.
	*/
	shared_mutex highPriorityMutex_;

	recursive_mutex drawableChunksWriteMutex_,
		            chunksMutex_,
		            freeChunksMutex_,
		            priorityMeshingListMutex_;
	condition_variable managerThreadCV_;
	condition_variable_any highPriorityUpdatesCV_;

	/*
	Used to force all meshing threads to synchronize with
	the rendering thread when a high priority
	chunk update is issued and it's imperative than the update
	made is reflected in the rendering thread as quickly as possible.
	*/
	atomic<bool> forceSyncFlag_;

};

inline const unordered_map<glm::vec3, chunk*>& chunkManager::chunks()
{

	return chunks_;

}

inline unordered_map<glm::vec3, vector<vertex>> const * chunkManager::drawableChunksRead() const
{

	return drawableChunksRead_;

}

inline unordered_map<glm::vec3, vector<vertex>>* chunkManager::drawableChunksWrite()
{

	return drawableChunksWrite_;

}

inline int chunkManager::nChunksToDraw() const 
{

	return nChunksToDraw_;

}

inline const unordered_set<glm::vec3>& chunkManager::freeableChunks() const
{

	return freeableChunks_;

}

inline const mutex& chunkManager::freeableChunksMutex() const
{

	return freeableChunksMutex_;

}

inline const mutex& chunkManager::managerThreadMutex() const
{

	return managerThreadMutex_;

}

inline const condition_variable& chunkManager::managerThreadCV() const
{

	return managerThreadCV_;

}

inline const atomic<bool>& chunkManager::forceSyncFlag() const
{

	return forceSyncFlag_;

}

inline recursive_mutex& chunkManager::chunksMutex()
{

	return chunksMutex_;

}

inline recursive_mutex& chunkManager::highPriorityListMutex()
{

	return priorityMeshingListMutex_;

}

inline unordered_set<glm::vec3>& chunkManager::freeableChunks()
{

	return freeableChunks_;

}

inline mutex& chunkManager::freeableChunksMutex()
{

	return freeableChunksMutex_;

}

inline mutex& chunkManager::managerThreadMutex()
{

	return managerThreadMutex_;

}

inline condition_variable& chunkManager::managerThreadCV()
{

	return managerThreadCV_;

}

inline atomic<bool>& chunkManager::forceSyncFlag()
{

	return forceSyncFlag_;

}

#endif