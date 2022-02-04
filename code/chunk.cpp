#include "chunk.h"
#include <cmath>
#include <cstddef>
#include "model.h"


// 'chunk' class.

chunk::chunk(chunkManager& chunkManager)
    : chunkManager_(chunkManager)
{

    renderingData_.chunkPos = glm::vec3(0, 0, 0);

    // Write chunk data section starts.

    {

        shared_lock<shared_mutex> lock(blocksMutex_);

        changed_ = true;

        setNBlocks(0);

        for (GLbyte x = 0; x < SCX; x++)
            for (GLbyte y = 0; y < SCY; y++)
                for (GLbyte z = 0; z < SCZ; z++)
                    setBlock(x, y, z, 0);

    }

    // Write chunk data section ends.

}

unsigned int chunk::getNBlocks() 
{

    return nBlocks_;

}

void chunk::setBlock(chunkRelativePos chunkRelPos, VoxelEng::block blockID)
{

    changed_ = true;

    if (blocks_[chunkRelPos.x][chunkRelPos.y][chunkRelPos.z] == 0 && blockID != 0)
        nBlocks_++;
    else
        if (blocks_[chunkRelPos.x][chunkRelPos.y][chunkRelPos.z] != 0 && blockID == 0 && nBlocks_ != 0)
            nBlocks_--;

    blocks_[chunkRelPos.x][chunkRelPos.y][chunkRelPos.z] = blockID;

}

void chunk::setBlock(GLbyte x, GLbyte y, GLbyte z, VoxelEng::block blockID)
{

    changed_ = true;

    if (blocks_[x][y][z] == 0 && blockID != 0)
        nBlocks_++;
    else
        if (blocks_[x][y][z] != 0 && blockID == 0 && nBlocks_ != 0)
            nBlocks_--;

    blocks_[x][y][z] = blockID;

}

void chunk::setNBlocks(unsigned int nBlocks)
{

    nBlocks_ = nBlocks;

}

void chunk::renewMesh()
{

    renderingData_.vertices.clear();
    if (nBlocks_)
    {

        VoxelEng::block blockID = 0;


        // Get information about neighbor chunks.
        std::vector<chunk*> neighborChunks = {

            chunkManager_.selectChunk(renderingData_.chunkPos.x, renderingData_.chunkPos.y, renderingData_.chunkPos.z + 1), // front
            chunkManager_.selectChunk(renderingData_.chunkPos.x, renderingData_.chunkPos.y, renderingData_.chunkPos.z - 1), // back
            chunkManager_.selectChunk(renderingData_.chunkPos.x, renderingData_.chunkPos.y + 1, renderingData_.chunkPos.z), // top
            chunkManager_.selectChunk(renderingData_.chunkPos.x, renderingData_.chunkPos.y - 1, renderingData_.chunkPos.z), // bottom
            chunkManager_.selectChunk(renderingData_.chunkPos.x + 1, renderingData_.chunkPos.y, renderingData_.chunkPos.z), // right
            chunkManager_.selectChunk(renderingData_.chunkPos.x - 1, renderingData_.chunkPos.y, renderingData_.chunkPos.z) // left

        };


        // Read chunk data section starts.

        // Lock neighbors' data.
        blocksMutex_.lock_shared();

        if (neighborChunks[0])
            neighborChunks[0]->blockDataMutex().lock_shared();

        if (neighborChunks[1])
            neighborChunks[1]->blockDataMutex().lock_shared();

        if (neighborChunks[2])
            neighborChunks[2]->blockDataMutex().lock_shared();

        if (neighborChunks[3])
            neighborChunks[3]->blockDataMutex().lock_shared();

        if (neighborChunks[4])
            neighborChunks[4]->blockDataMutex().lock_shared();

        if (neighborChunks[5])
            neighborChunks[5]->blockDataMutex().lock_shared();
       
        
        vertex aux;
        
        // Determine model from block's ID.
        for (VoxelEng::byte x = 0; x < SCX; x++)
            for (VoxelEng::byte y = 0; y < SCY; y++)
                for (VoxelEng::byte z = 0; z < SCZ; z++)
                {

                    // Add block's model to the mesh if necessary.
                    if (blockID = blocks_[x][y][z])
                    {

                        // Front face vertices with culling of non-visible faces.
                        if ((z < 15 && !blocks_[x][y][z + 1]) || (z == 15 && neighborChunks[0] && !neighborChunks[0]->blocks_[x][y][0]))
                        {

                            // Create the face's vertices.
                            for (int vertex = 0; vertex < VoxelEng::models::triangles_[0]->operator[](0).size(); vertex++)
                            {

                                aux.positions[0] = renderingData_.chunkPos.x * SCX + x + VoxelEng::models::models_[0]->operator[](VoxelEng::models::triangles_[0]->operator[](1)[vertex]).positions[0];
                                aux.positions[1] = renderingData_.chunkPos.y * SCY + y + VoxelEng::models::models_[0]->operator[](VoxelEng::models::triangles_[0]->operator[](1)[vertex]).positions[1];
                                aux.positions[2] = renderingData_.chunkPos.z * SCZ + z + VoxelEng::models::models_[0]->operator[](VoxelEng::models::triangles_[0]->operator[](1)[vertex]).positions[2];
                                renderingData_.vertices.push_back(aux);

                            }

                            // Add texture to the face.
                            VoxelEng::models::addTexture(blockID, blockID, renderingData_.vertices);

                        }

                        // Back face vertices with culling of non-visible faces.
                        if ((z > 0 && !blocks_[x][y][z - 1]) || (z == 0 && neighborChunks[1] && !neighborChunks[1]->blocks_[x][y][15]))
                        {

                            // Create the face's vertices
                            for (int vertex = 0; vertex < VoxelEng::models::triangles_[0]->operator[](0).size(); vertex++)
                            {

                                aux.positions[0] = renderingData_.chunkPos.x * SCX + x + VoxelEng::models::models_[0]->operator[](VoxelEng::models::triangles_[0]->operator[](0)[vertex]).positions[0];
                                aux.positions[1] = renderingData_.chunkPos.y * SCY + y + VoxelEng::models::models_[0]->operator[](VoxelEng::models::triangles_[0]->operator[](0)[vertex]).positions[1];
                                aux.positions[2] = renderingData_.chunkPos.z * SCZ + z + VoxelEng::models::models_[0]->operator[](VoxelEng::models::triangles_[0]->operator[](0)[vertex]).positions[2];
                                renderingData_.vertices.push_back(aux);

                            }

                            // Add texture to the face.
                            VoxelEng::models::addTexture(blockID, blockID, renderingData_.vertices);

                        }

                        // Top face vertices with culling of non-visible faces.
                        if ((y < 15 && !blocks_[x][y + 1][z]) || (y == 15 && neighborChunks[2] && !neighborChunks[2]->blocks_[x][0][z]))
                        {

                            // Create the face's vertices
                            for (int vertex = 0; vertex < VoxelEng::models::triangles_[0]->operator[](0).size(); vertex++)
                            {

                                aux.positions[0] = renderingData_.chunkPos.x * SCX + x + VoxelEng::models::models_[0]->operator[](VoxelEng::models::triangles_[0]->operator[](2)[vertex]).positions[0];
                                aux.positions[1] = renderingData_.chunkPos.y * SCY + y + VoxelEng::models::models_[0]->operator[](VoxelEng::models::triangles_[0]->operator[](2)[vertex]).positions[1];
                                aux.positions[2] = renderingData_.chunkPos.z * SCZ + z + VoxelEng::models::models_[0]->operator[](VoxelEng::models::triangles_[0]->operator[](2)[vertex]).positions[2];
                                renderingData_.vertices.push_back(aux);

                            }

                            // Add texture to the face.
                            VoxelEng::models::addTexture(blockID, blockID, renderingData_.vertices);

                        }

                        // Bottom face vertices with culling of non-visible faces.
                        if ((y > 0 && !blocks_[x][y - 1][z]) || (y == 0 && neighborChunks[3] && !neighborChunks[3]->blocks_[x][0][z]))
                        {

                            // Create the face's vertices
                            for (int vertex = 0; vertex < VoxelEng::models::triangles_[0]->operator[](0).size(); vertex++)
                            {

                                aux.positions[0] = renderingData_.chunkPos.x * SCX + x + VoxelEng::models::models_[0]->operator[](VoxelEng::models::triangles_[0]->operator[](3)[vertex]).positions[0];
                                aux.positions[1] = renderingData_.chunkPos.y * SCY + y + VoxelEng::models::models_[0]->operator[](VoxelEng::models::triangles_[0]->operator[](3)[vertex]).positions[1];
                                aux.positions[2] = renderingData_.chunkPos.z * SCZ + z + VoxelEng::models::models_[0]->operator[](VoxelEng::models::triangles_[0]->operator[](3)[vertex]).positions[2];
                                renderingData_.vertices.push_back(aux);

                            }

                            // Add texture to the face.
                            VoxelEng::models::addTexture(blockID, blockID, renderingData_.vertices);

                        }

                        // Right face vertices with culling of non-visible faces.
                        if ((x < 15 && !blocks_[x + 1][y][z]) || (x == 15 && neighborChunks[4] && !neighborChunks[4]->blocks_[0][y][z]))
                        {

                            // Create the face's vertices
                            for (int vertex = 0; vertex < VoxelEng::models::triangles_[0]->operator[](0).size(); vertex++)
                            {

                                aux.positions[0] = renderingData_.chunkPos.x * SCX + x + VoxelEng::models::models_[0]->operator[](VoxelEng::models::triangles_[0]->operator[](5)[vertex]).positions[0];
                                aux.positions[1] = renderingData_.chunkPos.y * SCY + y + VoxelEng::models::models_[0]->operator[](VoxelEng::models::triangles_[0]->operator[](5)[vertex]).positions[1];
                                aux.positions[2] = renderingData_.chunkPos.z * SCZ + z + VoxelEng::models::models_[0]->operator[](VoxelEng::models::triangles_[0]->operator[](5)[vertex]).positions[2];
                                renderingData_.vertices.push_back(aux);

                            }

                            // Add texture to the face.
                            VoxelEng::models::addTexture(blockID, blockID, renderingData_.vertices);

                        }

                        // Left face vertices with culling of non-visible faces.
                        if ((x > 0 && !blocks_[x - 1][y][z]) || (x == 0 && neighborChunks[5] && !neighborChunks[5]->blocks_[15][y][z]))
                        {

                            // Create the face's vertices
                            for (int vertex = 0; vertex < VoxelEng::models::triangles_[0]->operator[](0).size(); vertex++)
                            {

                                aux.positions[0] = renderingData_.chunkPos.x * SCX + x + VoxelEng::models::models_[0]->operator[](VoxelEng::models::triangles_[0]->operator[](4)[vertex]).positions[0];
                                aux.positions[1] = renderingData_.chunkPos.y * SCY + y + VoxelEng::models::models_[0]->operator[](VoxelEng::models::triangles_[0]->operator[](4)[vertex]).positions[1];
                                aux.positions[2] = renderingData_.chunkPos.z * SCZ + z + VoxelEng::models::models_[0]->operator[](VoxelEng::models::triangles_[0]->operator[](4)[vertex]).positions[2];
                                renderingData_.vertices.push_back(aux);

                            }

                            // Add texture to the face.
                            VoxelEng::models::addTexture(blockID, blockID, renderingData_.vertices);

                        }

                    }

                }

 
        // Unlock neighbors' data.
        blocksMutex_.unlock_shared();

        if (neighborChunks[0])
            neighborChunks[0]->blockDataMutex().unlock_shared();

        if (neighborChunks[1])
            neighborChunks[1]->blockDataMutex().unlock_shared();

        if (neighborChunks[2])
            neighborChunks[2]->blockDataMutex().unlock_shared();

        if (neighborChunks[3])
            neighborChunks[3]->blockDataMutex().unlock_shared();

        if (neighborChunks[4])
            neighborChunks[4]->blockDataMutex().unlock_shared();

        if (neighborChunks[5])
            neighborChunks[5]->blockDataMutex().unlock_shared();
            
        // Read chunk data section ends.

    }

}

chunkManager::chunkManager(int nChunksToDraw, const camera& playerCamera)
    : nChunksToDraw_(nChunksToDraw), playerCamera_(playerCamera), 
      drawableChunksRead_(new unordered_map<glm::vec3, vector<vertex>>),
      drawableChunksWrite_(new unordered_map<glm::vec3, vector<vertex>>),
      forceSyncFlag_(false)
{}

VoxelEng::block chunkManager::getBlock(const glm::vec3& pos)
{

    glm::vec3 chunkPos(trunc(pos.x / SCX), trunc(pos.y / SCY), trunc(pos.z / SCZ));
    chunk* selectedChunk;
    VoxelEng::block selectedBlock;


    unique_lock<recursive_mutex> lock(chunksMutex_);

    if (chunks_.find(chunkPos) != chunks_.end())
    {

        selectedChunk = chunks_.at(chunkPos);
        selectedBlock = selectedChunk->getBlock(static_cast<int>(pos.x > 0 ? pos.x : -pos.x) % SCX, 
                                                static_cast<int>(pos.y > 0 ? pos.y : -pos.y) % SCY,
                                                static_cast<int>(pos.z > 0 ? pos.z : -pos.z) % SCZ);

    }
    else
    {

        selectedBlock = 0;

    }

    return selectedBlock;

}

chunk* chunkManager::selectChunk(GLbyte x, GLbyte y, GLbyte z)
{

    glm::vec3 chunkPos(x, y, z);

    unique_lock<recursive_mutex> lock(chunksMutex_);

    if (chunks_.find(chunkPos) != chunks_.end())
        return chunks_.at(chunkPos);
    else
        return nullptr;

}

chunk* chunkManager::selectChunkByChunkPos(const glm::vec3& chunkPos)
{

    unique_lock<recursive_mutex> lock(chunksMutex_);

    if (chunks_.find(chunkPos) != chunks_.end())
        return chunks_.at(chunkPos);
    else
        return nullptr; 

}

chunk* chunkManager::selectChunkByRealPos(const glm::vec3& pos)
{

    glm::vec3 chunkPos(trunc(pos.x / SCX), trunc(pos.y / SCY), trunc(pos.z / SCZ));


    unique_lock<recursive_mutex> lock(chunksMutex_);

    if (chunks_.find(chunkPos) != chunks_.end())
        return chunks_.at(chunkPos);
    else
        return nullptr;

}

chunk* chunkManager::neighborMinusX(const glm::vec3& chunkPos)
{

	glm::vec3 neighborChunkPos(chunkPos.x - 1, chunkPos.y, chunkPos.z);

	unique_lock<recursive_mutex> lock(chunksMutex_);

	if (chunks_.find(neighborChunkPos) != chunks_.end())
		return chunks_.at(neighborChunkPos);
	else
		return nullptr;

}

chunk* chunkManager::neighborPlusX(const glm::vec3& chunkPos)
{

	glm::vec3 neighborChunkPos(chunkPos.x + 1, chunkPos.y, chunkPos.z);

	unique_lock<recursive_mutex> lock(chunksMutex_);

	if (chunks_.find(neighborChunkPos) != chunks_.end())
		return chunks_.at(neighborChunkPos);
	else
		return nullptr;

}

chunk* chunkManager::neighborMinusY(const glm::vec3& chunkPos)
{

	glm::vec3 neighborChunkPos(chunkPos.x, chunkPos.y - 1, chunkPos.z);

	unique_lock<recursive_mutex> lock(chunksMutex_);

	if (chunks_.find(neighborChunkPos) != chunks_.end())
		return chunks_.at(neighborChunkPos);
	else
		return nullptr;

}

chunk* chunkManager::neighborPlusY(const glm::vec3& chunkPos)
{

	glm::vec3 neighborChunkPos(chunkPos.x, chunkPos.y + 1, chunkPos.z);

	unique_lock<recursive_mutex> lock(chunksMutex_);

	if (chunks_.find(neighborChunkPos) != chunks_.end())
		return chunks_.at(neighborChunkPos);
	else
		return nullptr;

}

chunk* chunkManager::neighborMinusZ(const glm::vec3& chunkPos)
{

	glm::vec3 neighborChunkPos(chunkPos.x, chunkPos.y, chunkPos.z - 1);

	unique_lock<recursive_mutex> lock(chunksMutex_);

	if (chunks_.find(neighborChunkPos) != chunks_.end())
		return chunks_.at(neighborChunkPos);
	else
		return nullptr;

}

chunk* chunkManager::neighborPlusZ(const glm::vec3& chunkPos)
{

	glm::vec3 neighborChunkPos(chunkPos.x, chunkPos.y, chunkPos.z + 1);

	unique_lock<recursive_mutex> lock(chunksMutex_);

	if (chunks_.find(neighborChunkPos) != chunks_.end())
		return chunks_.at(neighborChunkPos);
	else
		return nullptr;

}

void chunkManager::pushDrawableChunks(const chunkRenderingData& renderingData)
{

    unique_lock<recursive_mutex> lock(drawableChunksWriteMutex_);

    drawableChunksWrite_->insert_or_assign(renderingData.chunkPos, renderingData.vertices);

}

void chunkManager::swapDrawableChunksLists() 
{

    unordered_map<glm::vec3, vector<vertex>>* aux = drawableChunksRead_;

    drawableChunksRead_ = drawableChunksWrite_;
    drawableChunksWrite_ = aux;

}

void chunkManager::updatePriorityChunks()
{

    glm::vec3 chunkPos;


    while(!priorityUpdateList_.empty())
    { 

        chunkPos = priorityUpdateList_.front();

        priorityUpdateList_.pop_front();

        if (drawableChunksWrite_->contains(chunkPos))
            drawableChunksRead_->insert_or_assign(chunkPos, drawableChunksWrite_->at(chunkPos));

    }

}

void chunkManager::loadChunk(const glm::vec3& chunkPos)
{

    bool chunkNotLoaded;


    {
    
        unique_lock<recursive_mutex> lock(chunksMutex_);

        chunkNotLoaded = chunks_.find(chunkPos) == chunks_.end();
    
    }

    if (chunkNotLoaded)
    {

        chunk* chunkPtr = nullptr;

        {

            unique_lock<recursive_mutex> lock(freeChunksMutex_);

            if (freeChunks_.size())
            {

                chunkPtr = freeChunks_.front();
                freeChunks_.pop_front();

            }
            else
                chunkPtr = new chunk(*this);

        }

        chunkPtr->changed() = true;
        chunkPtr->chunkPos() = chunkPos;

        // Terrain generation/loading.
        for (GLbyte x = 0; x < SCX; x++)
            for (GLbyte y = 0; y < SCY; y++)
                for (GLbyte z = 0; z < SCZ; z++)
                    chunkPtr->setBlock(x, y, z, (chunkPos.y <= 8) * (rand() % 3 + 1));

        {

            unique_lock<recursive_mutex> lock(chunksMutex_);

            chunks_.insert_or_assign(chunkPos, chunkPtr);

        }

    }
     
}

void chunkManager::unloadChunk(const glm::vec3& chunkPos)
{

    unordered_map<glm::vec3, chunk*>::iterator it;


    if ((it = chunks_.find(chunkPos)) != chunks_.end())
    {

        chunk* unloadedChunk = it->second;
        chunks_.erase(chunkPos);
        freeChunks_.push_back(unloadedChunk);
    
    }

}

// TODO.
// REDUCE THE ARGUMENT NUMBER. TRY TO REFACTOR SOME
// OF THESE PARAMETERS AS MEMBERS OF chunkManager CLASS.
void chunkManager::meshChunks(const atomic<bool>& appFinished, const atomic<int>& chunkRange,
                              int rangeStart, int rangeEnd, shared_mutex& syncMutex, condition_variable_any& meshingThreadsCV,
                              atomic<bool>& meshingTsCVContinue, barrier<>& syncPoint)
{

    {

        shared_lock syncLock(syncMutex);

        glm::vec3 playerChunkCoord,
                  viewedChunkCoord,
                  offset;
        chunk* selectedChunk = nullptr;


        while (!appFinished)
        {

            // Wait for the chunk management thread's signal.
            while (!meshingTsCVContinue)
                meshingThreadsCV.wait(syncLock);

            playerChunkCoord = playerCamera_.chunkPos();

            // World loading.
            for (offset.y = rangeStart; offset.y <= rangeEnd; offset.y++)
                for (offset.x = -chunkRange; offset.x <= chunkRange; offset.x++)
                    for (offset.z = -chunkRange; offset.z <= chunkRange; offset.z++)
                    {

                        // First check if there is any high priority chunk update.
                        // If so, force synchronization with the
                        // rendering thread to reflect the change made.
                        {

                            unique_lock<recursive_mutex> priorityListLock(priorityMeshingListMutex_);
                            forceSyncFlag_ = !priorityMeshingList_.empty();
  
                        }

                        // If a forcible synchronization was issued.
                        if (forceSyncFlag_)
                        {

                            // Sync with the other meshing threads and the chunk management thread.
                            syncPoint.arrive_and_wait();
                            meshingTsCVContinue = false;

                            // Wait for the chunk management thread's signal.
                            while (!meshingTsCVContinue)
                                meshingThreadsCV.wait(syncLock);

                        }

                        // Now continue processing common chunk updates.
                        viewedChunkCoord = playerChunkCoord + offset;
                        selectedChunk = selectChunkByChunkPos(viewedChunkCoord);

                        if (selectedChunk) // If that chunk is loaded.
                        {

                            // Unmark as freeable.
                            freeableChunksMutex_.lock();
                            freeableChunks_.erase(viewedChunkCoord);
                            freeableChunksMutex_.unlock();

                            // Regenerate mesh and push for rendering if necessary. 
                            if (selectedChunk->getNBlocks())
                            {

                                if (selectedChunk->changed())
                                {

                                    selectedChunk->changed() = false;
                                    selectedChunk->renewMesh();

                                }

                                pushDrawableChunks(selectedChunk->renderingData());

                            }

                        }
                        else
                            loadChunk(viewedChunkCoord);

                    }

            // Sync with the other meshing threads and the chunk management thread.
            syncPoint.arrive_and_wait();
            meshingTsCVContinue = false;

        }

    }

    // Finish procedure.
    syncPoint.arrive_and_drop();

}

void chunkManager::manageChunks(const atomic<bool>& app_finished, unsigned int nMeshingThreads)
{

    vector<thread> meshingThreads;


    {

        unique_lock<mutex> lock(managerThreadMutex_);


        // The total number of chunks rendered in the Y-axis.
        // This value is different from the values in the
        // other axes to prevent slow chunk loading while
        // maintaining a good render distance range.
        int totalYChunks = Y_CHUNKS_RANGE * 2 + 1;
        int rangeStart,
            rangeEnd;
        atomic<int> chunkRange = 0;
        atomic<bool> meshingTsCVContinue = false;
        shared_mutex syncMutex;
        condition_variable_any meshingThreadsCV;
        barrier syncPoint(nMeshingThreads + 1);
        chunk* priorityChunk = nullptr;


        // Initialice meshing threads.
        for (int i = 0; i < nMeshingThreads; i++)
        {

            rangeStart = totalYChunks / nMeshingThreads * i;
            if (i == nMeshingThreads - 1)
                rangeEnd = totalYChunks - 1;
            else
                rangeEnd = rangeStart + totalYChunks / nMeshingThreads - 1;

            meshingThreads.push_back(thread(&chunkManager::meshChunks, this, 
                                            ref(app_finished), ref(chunkRange),
                                            rangeStart - Y_CHUNKS_RANGE, rangeEnd - Y_CHUNKS_RANGE,
                                            ref(syncMutex), ref(meshingThreadsCV),
                                            ref(meshingTsCVContinue), ref(syncPoint)));

        }

        // Chunk management main loop.
        while (!app_finished)
        {

            // This will only execute when a high priority chunk update
            // is not issued.
            if (!forceSyncFlag_)
            { 

                // Marks all chunks as freeable
                for (unordered_map<glm::vec3, chunk*>::const_iterator it = chunks().cbegin(); it != chunks().cend(); it++)
                    freeableChunks_.insert(it->first);

            }
            else // Reset the forcible synchronization flag.
                forceSyncFlag_ = false;
                
            // Signal all meshing threads to begin.
            meshingTsCVContinue = true;
            meshingThreadsCV.notify_all();

            // Wait for all meshing threads to finish.
            syncPoint.arrive_and_wait();


            // This will only execute when a high priority chunk update
            // is not issued.
            // This prevents unloading chunks that are within the player's view range
            // and that didn't get viewed because search stopped because
            // a high priority chunk update was issued. It also prevents
            if (!forceSyncFlag_) {

                // All remaining chunks marked as freeable are freed.
                // No thread can access the chunks dictionary  
                // at the same time this is being executed.
                chunksMutex_.lock();
                for (unordered_set<glm::vec3>::iterator it = freeableChunks_.begin(); it != freeableChunks_.end(); it++)
                    unloadChunk(*it);
                chunksMutex_.unlock();

                // Increase chunk viewing range for the next
                // iteration until we reach the limit established
                // by the player's configuration.
                if (chunkRange <= nChunksToDraw())
                    chunkRange++;

                // Sync with the rendering thread.
                managerThreadCV_.wait(lock);

                // Reset some data structures for the next iteration.
                freeableChunks_.clear();
                drawableChunksWrite_->clear();

            }
            else { // If there is a high priority chunk update, process it.
            
                glm::vec3 priorityChunkPos;
                bool synchronize = false,
                     isLockActive = false;


                priorityMeshingListMutex_.lock();
                isLockActive = true;

                while (!priorityMeshingList_.empty()) {

                    priorityChunkPos = priorityMeshingList_.front();
                    priorityMeshingList_.pop_front();

                    priorityMeshingListMutex_.unlock();
                    isLockActive = false;

                    priorityChunk = selectChunkByChunkPos(priorityChunkPos);

                    if (priorityChunk) {

                        synchronize = true;

                        // Unmark as freeable.
                        freeableChunksMutex_.lock();
                        freeableChunks_.erase(priorityChunk->chunkPos());
                        freeableChunksMutex_.unlock();

                        // Update mesh. 
                        priorityChunk->changed() = false;
                        priorityChunk->renewMesh();

                        pushDrawableChunks(priorityChunk->renderingData());
                        priorityUpdateList_.push_back(priorityChunk->chunkPos());

                    }

                    priorityMeshingListMutex_.lock();
                    isLockActive = true;

                }

                if(isLockActive)
                    priorityMeshingListMutex_.unlock();
                
                // Sync with the rendering thread if necessary.
                if(synchronize)
                    managerThreadCV_.wait(lock);

            }

        }

        // Finish procedure.
        // First unblock any meshing thread so they can finish their execution properly.
        syncPoint.arrive_and_drop();
        meshingTsCVContinue = true;
        meshingThreadsCV.notify_all();
        // Now wait for all meshing threads to end.
        for (int i = 0; i < nMeshingThreads; i++)
            meshingThreads[i].join();

    }

}

void chunkManager::highPriorityUpdate(const glm::vec3& chunkPos)
{

    chunk* selectedChunk = selectChunkByChunkPos(chunkPos);


    unique_lock<recursive_mutex> lock(priorityMeshingListMutex_);

    if (selectedChunk)
        priorityMeshingList_.push_back(chunkPos);

}

chunkManager::~chunkManager()
{

    for (unordered_map<glm::vec3, chunk*>::iterator it = chunks_.begin(); it != chunks_.end(); it++)
        if (it->second)
            delete it->second;

    for (deque<chunk*>::iterator it = freeChunks_.begin(); it != freeChunks_.end(); it++)
        if (*it)
            delete *it;

    delete drawableChunksRead_;
    delete drawableChunksWrite_;

}