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
using namespace std;


//////////////////////////////
//Forward class declarations//
//////////////////////////////

class camera;
class chunkManager;


///////////
//Classes//
///////////

/*
Wraps up data used to render a chunk.
*/
struct chunkRenderingData
{

	glm::vec3 chunkPos;
	vector<vertex> vertices;

};

// TODO. ADD DOCUMENTATION TO THE NEW ADDED METHODS.
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
	block getBlock(GLbyte x, GLbyte y, GLbyte z);

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

	/*
	True if the chunk is dirty and false otherwise.
	*/
	bool getIsChanged();

	const atomic<bool>& changedHighPriority() const;


	// Modifiers.

	/*
	Sets the value of a block within the chunk.
	The chunk is marked as dirty.
	*/
	void setBlock(GLbyte x, GLbyte y, GLbyte z, block block_id);

	glm::vec3& chunkPos();

	void setNBlocks(unsigned int nBlocks);

	shared_mutex& blockDataMutex();


	/*
	Mark chunk as dirty (true) or not (false)
	*/
	void setIsChanged(bool isChanged);


	/*
	Regenerate the chunk's mesh.
	*/
	void renewMesh();
	
private:

	block blocks_[16][16][16];
	atomic<bool> changed_;
	atomic<unsigned int> nBlocks_;
	chunkRenderingData renderingData_;
	chunkManager& chunkManager_;
	shared_mutex blocksMutex_;

};

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

inline const chunkRenderingData& chunk::renderingData() const
{

	return renderingData_;

}

inline shared_mutex& chunk::blockDataMutex()
{

	return blocksMutex_;

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

	deque<chunkRenderingData> const* drawableChunksRead() const;

	int nChunksToDraw() const;

	block getBlock(const glm::vec3& pos);

	const unordered_set<glm::vec3>& freeableChunks() const;

	const mutex& freeableChunksMutex() const;

	const mutex& managerThreadMutex() const;

	const condition_variable& managerThreadCV() const;


	// Modifiers.

	chunk* selectChunk(GLbyte x, GLbyte y, GLbyte z);

	chunk* selectChunkByChunkPos(const glm::vec3& chunkPos);

	/*
	Uses real float point coordinates.
	*/
	chunk* selectChunkByRealPos(const glm::vec3& pos);

	/*
	WARNING. This operation is not thread-safe.
	To push back a chunk's rendering data into this deque,
	use chunkManager::pushDrawableChunks(...) method to prevent
	race conditions when using multiple threads in the
	chunk management system.
	*/
	deque<chunkRenderingData>* drawableChunksWrite();

	recursive_mutex& chunksMutex();

	unordered_set<glm::vec3>& freeableChunks();

	mutex& freeableChunksMutex();

	mutex& managerThreadMutex();

	condition_variable& managerThreadCV();


	// Other methods.

	/*
	All neigbor chunks from the one at 'chunkPos' will be marked as changed and will have
	their respective meshes regenerated asynchronously.
	Use this when, for example, a player removes a block at the edge of the chunk at 'chunkPos'.
	This is requires an access to where the loaded chunks are stored. Therefore, this operation must
	be done atomically in relation to any other operations that attempt to access to said data.
	*/
	void forceNeighborsToUpdate(const glm::vec3& chunkPos);

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
	deque<chunkRenderingData>* drawableChunksWrite_,
		                     * drawableChunksRead_;
	deque<chunk*> freeChunks_;
	unordered_set<glm::vec3> freeableChunks_;
	
	/* 
	Chunks with high update priority
	(for example, chunks modified by the player
	by removing or adding a block).
	*/
	deque<glm::vec3> highPriorityList_;

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
		            highPriorityListMutex_;
	condition_variable managerThreadCV_;
	condition_variable_any highPriorityUpdatesCV_;
	atomic<bool> highPriorityCVFlag_; // Delete this unused member.

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

inline deque<chunkRenderingData> const * chunkManager::drawableChunksRead() const
{

	return drawableChunksRead_;

}

inline deque<chunkRenderingData>* chunkManager::drawableChunksWrite()
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

inline recursive_mutex& chunkManager::chunksMutex()
{

	return chunksMutex_;

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

#endif