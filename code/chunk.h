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
#include "threadPool.h"


#include <iostream>
#include <ostream>
using namespace std;


///////////////
//Definitions//
///////////////

// How many chunks to load in the y-axis.
#define Y_CHUNKS_RANGE 12
// Chunk size in X axis.
#define SCX 16
// Chunk size in Y axis.
#define SCY 16
// Chunk size in Z axis.
#define SCZ 16
// Block's id.
typedef unsigned short Cube;

//////////////////////////////
//Forward class declarations//
//////////////////////////////
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
	vector<Block_vertex> vertices;

};

/*
Represents a section of the voxel world, with its blocks, mesh, position and other infomation.
*/
class Chunk
{

public:

	Chunk(chunkManager& chunkManager);

	void set(GLbyte x, GLbyte y, GLbyte z, Cube block_id);
	void renewMesh();

	Cube get(GLbyte x, GLbyte y, GLbyte z);

	GLbyte x() const;
	GLbyte y() const;
	GLbyte z() const;
	const glm::vec3& chunkPos() const;
	glm::vec3& chunkPos();
	const glm::vec3& pos() const;
	const vector<Block_vertex>& vertices() const;
	const chunkRenderingData& renderingData() const;

	unsigned int nBlocks() const;
	unsigned int& nBlocks();
	bool changed() const;
	bool& changed();
	bool hasBlocks() const;

	~Chunk();
	
private:

	Cube blocks_[16][16][16];
	bool changed_;
	unsigned int nBlocks_;
	chunkRenderingData renderingData_;
	chunkManager& chunkManager_;

};

inline Cube Chunk::get(GLbyte x, GLbyte y, GLbyte z)
{

	return blocks_[x][y][z];

}

inline GLbyte Chunk::x() const
{

	return renderingData_.chunkPos.x;

}

inline GLbyte Chunk::y() const
{

	return renderingData_.chunkPos.y;

}

inline GLbyte Chunk::z() const
{

	return renderingData_.chunkPos.z;

}

inline const glm::vec3& Chunk::chunkPos() const 
{

	return renderingData_.chunkPos;

}

inline glm::vec3& Chunk::chunkPos()
{

	return renderingData_.chunkPos;
	
}

inline const glm::vec3& Chunk::pos() const
{

	return glm::vec3(renderingData_.chunkPos.x * SCX, renderingData_.chunkPos.y * SCY, renderingData_.chunkPos.z * SCZ);

}

inline const vector<Block_vertex>& Chunk::vertices() const
{

	return renderingData_.vertices;

}

inline const chunkRenderingData& Chunk::renderingData() const
{

	return renderingData_;

}

inline unsigned int Chunk::nBlocks() const
{

	return nBlocks_;

}

inline unsigned int& Chunk::nBlocks()
{

	return nBlocks_;

}

inline bool Chunk::changed() const
{

	return changed_;

}

inline bool& Chunk::changed()
{

	return changed_;

}

inline bool Chunk::hasBlocks() const 
{

	return nBlocks_;

}


/*
Used for managing the chunks' life cycle.
*/
class chunkManager
{

public:

	chunkManager(int nChunksToDraw, const Camera& playerCamera);


	// Observers.

	const unordered_map<glm::vec3, Chunk*>& chunks();
	deque<chunkRenderingData> const* drawableChunksRead() const;
	const deque<glm::vec3>& meshRenewQueue() const;
	int nChunksToDraw() const;

	// Modifiers.

	Chunk* chunk(GLbyte x, GLbyte y, GLbyte z);
	Chunk* chunk(const glm::vec3& chunkPos);
	deque<chunkRenderingData>* drawableChunksWrite();
	deque<glm::vec3>& meshRenewQueue();


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
	Function called by meshing threads.
	*/
	void meshChunks(const atomic<bool>& app_finished, const atomic<int>& chunkRange,
					int rangeStart, int rangeEnd,
					unordered_set<glm::vec3>& freeableChunks, mutex& freeableChunksMutex,
					shared_mutex& syncMutex, condition_variable_any& meshingThreadsCV,
		            atomic<bool>& meshingTsCVContinue, barrier<>& syncPoint);

	/*
	Function called by the chunk management thread.
	It coordinates all meshing threads and manages the chunk unloading process
	in order to prevent any race condition between said threads, among other things
	such as synchronization and data transfering with the rendering thread.
	*/
	void manageChunks(const atomic<bool>& app_finished, unsigned int nMeshingThreads,
		mutex& managerThreadMutex, condition_variable& managerThreadCV);

	/*
	Frees any memory allocated in the process of generating the world, like
	all the Chunk objects created to load it.
	*/
	~chunkManager();

private:

	int nChunksToDraw_;
	unordered_map<glm::vec3, Chunk*> chunks_;
	deque<chunkRenderingData>* drawableChunksWrite_,
		                     * drawableChunksRead_;
	deque<Chunk*> freeChunks_;
	deque<glm::vec3> chunksToUpdate_;
	const Camera& playerCamera_;
	recursive_mutex drawableChunksWriteMutex_,
		            chunksMutex_,
		            freeChunksMutex_;

};

inline const unordered_map<glm::vec3, Chunk*>& chunkManager::chunks()
{

	return chunks_;

}

inline deque<chunkRenderingData> const * chunkManager::drawableChunksRead() const
{

	return drawableChunksRead_;

}

/*
WARNING. This operation is not thread-safe.
To push back a chunk's rendering data into this deque,
use chunkManager::pushDrawableChunks(...) method to prevent
race conditions when using multiple threads in the
chunk management system.
*/
inline deque<chunkRenderingData>* chunkManager::drawableChunksWrite()
{

	return drawableChunksWrite_;

}

inline const deque<glm::vec3>& chunkManager::meshRenewQueue() const
{

	return chunksToUpdate_;

}

inline deque<glm::vec3>& chunkManager::meshRenewQueue()
{

	return chunksToUpdate_;

}

inline int chunkManager::nChunksToDraw() const 
{

	return nChunksToDraw_;

}

#endif