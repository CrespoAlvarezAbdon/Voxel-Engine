#include <cmath>
#include "chunk.h"

#include <iostream>
#include <ostream>
using namespace std;

// 'chunk' class.

chunk::chunk(chunkManager& chunkManager)
    : chunkManager_(chunkManager)
{

    renderingData_.chunkPos = glm::vec3(0, 0, 0);

    // Write chunk data section starts.

    {

        shared_lock<shared_mutex> lock(blocksMutex_);

        setIsChanged(true);

        setNBlocks(0);

        for (GLbyte x = 0; x < SCX; x++)
            for (GLbyte y = 0; y < SCY; y++)
                for (GLbyte z = 0; z < SCZ; z++)
                    setBlock(x, y, z, 0);

    }

    // Write chunk data section ends.

}

bool chunk::getIsChanged()
{

    return changed_;

}

void chunk::setIsChanged(bool isChanged)
{

    changed_ = isChanged;

}

block chunk::getBlock(GLbyte x, GLbyte y, GLbyte z)
{

    return blocks_[x][y][z];

}

unsigned int chunk::getNBlocks() 
{

    return nBlocks_;

}

void chunk::setBlock(GLbyte x, GLbyte y, GLbyte z, block blockID)
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

        float atlasWidth = texture::blockTextureAtlas()->width(),
            atlasHeight = texture::blockTextureAtlas()->height(),
            textureWidth = texture::blockAtlasResolution(),
            textureHeight = texture::blockAtlasResolution(),
            texCoordX,
            texCoordY,
            texCoordX2,
            texCoordY2;

        // Get information about neighbor chunks.
        chunk* frontNeighbor = chunkManager_.selectChunk(renderingData_.chunkPos.x, renderingData_.chunkPos.y, renderingData_.chunkPos.z + 1),
             * backNeighbor = chunkManager_.selectChunk(renderingData_.chunkPos.x, renderingData_.chunkPos.y, renderingData_.chunkPos.z - 1),
             * topNeighbor = chunkManager_.selectChunk(renderingData_.chunkPos.x, renderingData_.chunkPos.y + 1, renderingData_.chunkPos.z),
             * bottomNeighbor = chunkManager_.selectChunk(renderingData_.chunkPos.x, renderingData_.chunkPos.y - 1, renderingData_.chunkPos.z),
             * rightNeighbor = chunkManager_.selectChunk(renderingData_.chunkPos.x + 1, renderingData_.chunkPos.y, renderingData_.chunkPos.z),
             * leftNeighbor = chunkManager_.selectChunk(renderingData_.chunkPos.x - 1, renderingData_.chunkPos.y, renderingData_.chunkPos.z);

        // Read chunk data section starts.

        // Lock.
        blocksMutex_.lock_shared();

        if (frontNeighbor)
            frontNeighbor->blockDataMutex().lock_shared();

        if (backNeighbor)
            backNeighbor->blockDataMutex().lock_shared();

        if (topNeighbor)
            topNeighbor->blockDataMutex().lock_shared();

        if (bottomNeighbor)
            bottomNeighbor->blockDataMutex().lock_shared();

        if (rightNeighbor)
            rightNeighbor->blockDataMutex().lock_shared();

        if (leftNeighbor)
            leftNeighbor->blockDataMutex().lock_shared();


        for (GLbyte x = 0; x < SCX; x++)
            for (GLbyte y = 0; y < SCY; y++)
                for (GLbyte z = 0; z < SCZ; z++)
                {

                    // If a block's ID equals 0, it means that it is an empty block 
                    // and that it doesn't have any vertices to generate.
                    if (getBlock(x, y, z))
                    {

                        GLbyte xPlus1 = x + 1,
                            yPlus1 = y + 1,
                            zPlus1 = z + 1;

                        texCoordX = blocks_[x][y][z] / (atlasWidth / textureWidth);
                        texCoordY = (ceil(blocks_[x][y][z] / (atlasWidth / textureWidth))) / (atlasHeight / textureHeight);
                        texCoordX2 = texCoordX - 1 / ((atlasWidth / textureWidth));
                        texCoordY2 = texCoordY - 1 / ((atlasWidth / textureWidth));

                        // Vertices' generation and culling.
                        // We don't want to render the faces of blocks at the boundary of the z and x axis if the neighbor block belongs
                        // to an unloaded chunk or it isn't solid since the player is never going to see those faces because, 
                        // when the player tries to move there to see those faces, the neighbor chunks will be loaded. 
                        // This has two possible outcomes. 
                        // The first one is that the faces are rendered (because the neighboring block is transparent, like a glass block, for example)
                        // or that they aren't rendered because the neighbor block is solid and hides this face from the player.

                        // Front face vertices with culling of non-visible faces.
                        if ((z < 15 && !getBlock(x, y, zPlus1)) || (z == 15 && frontNeighbor && !frontNeighbor->getBlock(x, y, 0)))
                        {

                            renderingData_.vertices.push_back(vertex{ x,      y,      zPlus1, 0, texCoordX , texCoordY });
                            renderingData_.vertices.push_back(vertex{ xPlus1, y,      zPlus1, 0, texCoordX2, texCoordY });
                            renderingData_.vertices.push_back(vertex{ xPlus1, yPlus1, zPlus1, 0, texCoordX2, texCoordY2 });
                            renderingData_.vertices.push_back(vertex{ xPlus1, yPlus1, zPlus1, 0, texCoordX2, texCoordY2 });
                            renderingData_.vertices.push_back(vertex{ x,      yPlus1, zPlus1, 0, texCoordX , texCoordY2 });
                            renderingData_.vertices.push_back(vertex{ x,      y,      zPlus1, 0, texCoordX , texCoordY });

                        }

                        // Back face vertices with culling of non-visible faces.
                        if ((z > 0 && !getBlock(x, y, z - 1)) || (z == 0 && backNeighbor && !backNeighbor->getBlock(x, y, 15)))
                        {

                            renderingData_.vertices.push_back(vertex{ xPlus1 , y,      z      , 0, texCoordX , texCoordY });
                            renderingData_.vertices.push_back(vertex{ x      , y,      z      , 0, texCoordX2, texCoordY });
                            renderingData_.vertices.push_back(vertex{ x      , yPlus1, z      , 0, texCoordX2, texCoordY2 });
                            renderingData_.vertices.push_back(vertex{ x      , yPlus1, z      , 0, texCoordX2, texCoordY2 });
                            renderingData_.vertices.push_back(vertex{ xPlus1 , yPlus1, z      , 0, texCoordX , texCoordY2 });
                            renderingData_.vertices.push_back(vertex{ xPlus1 , y,      z      , 0, texCoordX , texCoordY });

                        }

                        // Top face vertices with culling of non-visible faces.
                        if ((y < 15 && !getBlock(x, yPlus1, z)) || (y == 15 && topNeighbor && !topNeighbor->getBlock(x, 0, z)))
                        {

                            renderingData_.vertices.push_back(vertex{ x      , yPlus1, zPlus1 , 0, texCoordX , texCoordY });
                            renderingData_.vertices.push_back(vertex{ xPlus1 , yPlus1, zPlus1 , 0, texCoordX2, texCoordY });
                            renderingData_.vertices.push_back(vertex{ xPlus1 , yPlus1, z      , 0, texCoordX2, texCoordY2 });
                            renderingData_.vertices.push_back(vertex{ xPlus1 , yPlus1, z      , 0, texCoordX2, texCoordY2 });
                            renderingData_.vertices.push_back(vertex{ x      , yPlus1, z      , 0, texCoordX , texCoordY2 });
                            renderingData_.vertices.push_back(vertex{ x      , yPlus1, zPlus1 , 0, texCoordX , texCoordY });

                        }

                        // Bottom face vertices with culling of non-visible faces.
                        if ((y > 0 && !getBlock(x, y - 1, z)) || (y == 0 && bottomNeighbor && !bottomNeighbor->getBlock(x, 0, z)))
                        {

                            renderingData_.vertices.push_back(vertex{ x     , y      , z,      0, texCoordX , texCoordY });
                            renderingData_.vertices.push_back(vertex{ xPlus1, y      , z,      0, texCoordX2, texCoordY });
                            renderingData_.vertices.push_back(vertex{ xPlus1, y      , zPlus1, 0, texCoordX2, texCoordY2 });
                            renderingData_.vertices.push_back(vertex{ xPlus1, y      , zPlus1, 0, texCoordX2, texCoordY2 });
                            renderingData_.vertices.push_back(vertex{ x     , y      , zPlus1, 0, texCoordX , texCoordY2 });
                            renderingData_.vertices.push_back(vertex{ x     , y      , z,      0, texCoordX , texCoordY });

                        }

                        // Right face vertices with culling of non-visible faces.
                        if ((x < 15 && !getBlock(xPlus1, y, z)) || (x == 15 && rightNeighbor && !rightNeighbor->getBlock(0, y, z)))
                        {

                            renderingData_.vertices.push_back(vertex{ xPlus1 , y     , zPlus1, 0, texCoordX , texCoordY });
                            renderingData_.vertices.push_back(vertex{ xPlus1 , y     , z     , 0, texCoordX2, texCoordY });
                            renderingData_.vertices.push_back(vertex{ xPlus1 , yPlus1, z     , 0, texCoordX2, texCoordY2 });
                            renderingData_.vertices.push_back(vertex{ xPlus1 , yPlus1, z     , 0, texCoordX2, texCoordY2 });
                            renderingData_.vertices.push_back(vertex{ xPlus1 , yPlus1, zPlus1, 0, texCoordX , texCoordY2 });
                            renderingData_.vertices.push_back(vertex{ xPlus1 , y     , zPlus1, 0, texCoordX , texCoordY });

                        }

                        // Left face vertices with culling of non-visible faces.
                        if ((x > 0 && !getBlock(x - 1, y, z)) || (x == 0 && leftNeighbor && !leftNeighbor->getBlock(15, y, z)))
                        {

                            renderingData_.vertices.push_back(vertex{ x     , y     , z     ,  0, texCoordX , texCoordY });
                            renderingData_.vertices.push_back(vertex{ x     , y     , zPlus1,  0, texCoordX2, texCoordY });
                            renderingData_.vertices.push_back(vertex{ x     , yPlus1, zPlus1,  0, texCoordX2, texCoordY2 });
                            renderingData_.vertices.push_back(vertex{ x     , yPlus1, zPlus1,  0, texCoordX2, texCoordY2 });
                            renderingData_.vertices.push_back(vertex{ x     , yPlus1, z     ,  0, texCoordX , texCoordY2 });
                            renderingData_.vertices.push_back(vertex{ x     , y     , z     ,  0, texCoordX , texCoordY });

                        }

                    }

                }
 
        // Unlock.
        blocksMutex_.unlock_shared();

        if (frontNeighbor)
            frontNeighbor->blockDataMutex().unlock_shared();

        if (backNeighbor)
            backNeighbor->blockDataMutex().unlock_shared();

        if (topNeighbor)
            topNeighbor->blockDataMutex().unlock_shared();

        if (bottomNeighbor)
            bottomNeighbor->blockDataMutex().unlock_shared();

        if (rightNeighbor)
            rightNeighbor->blockDataMutex().unlock_shared();

        if (leftNeighbor)
            leftNeighbor->blockDataMutex().unlock_shared();
            
        // Read chunk data section ends.

    }

}

chunkManager::chunkManager(int nChunksToDraw, const camera& playerCamera)
    : nChunksToDraw_(nChunksToDraw), playerCamera_(playerCamera), 
      drawableChunksRead_(new deque<chunkRenderingData>), drawableChunksWrite_(new deque<chunkRenderingData>),
      highPriorityCVFlag_(false), forceSyncFlag_(false)
{}

block chunkManager::getBlock(const glm::vec3& pos) 
{

    glm::vec3 chunkPos(trunc(pos.x / SCX), trunc(pos.y / SCY), trunc(pos.z / SCZ));
    chunk* selectedChunk;
    block selectedBlock;


    unique_lock<recursive_mutex> lock(chunksMutex_);

    if (chunks_.find(chunkPos) != chunks_.end())
    {

        selectedChunk = chunks_.at(chunkPos);
        selectedBlock = selectedChunk->getBlock(static_cast<int>(pos.x) % SCX, 
                                                static_cast<int>(pos.y) % SCY, 
                                                static_cast<int>(pos.z) % SCZ);

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

// TODO.
// UPDATE THIS METHOD.
// ADD MUTUAL EXCLUSION WHEN NECESSARY
void chunkManager::forceNeighborsToUpdate(const glm::vec3& chunkPos)
{

    unordered_map<glm::vec3, chunk*>::iterator it;

    // +x neighbor.
    it = chunks_.find(chunkPos + glm::vec3(1, 0, 0));
    if (it != chunks_.end())
        it->second->setIsChanged(true);

    // -x neighbor.
    it = chunks_.find(chunkPos + glm::vec3(-1, 0, 0));
    if (it != chunks_.end())
        it->second->setIsChanged(true);

    // +y neighbor.
    it = chunks_.find(chunkPos + glm::vec3(0, 1, 0));
    if (it != chunks_.end())
        it->second->setIsChanged(true);

    // -y neighbor.
    it = chunks_.find(chunkPos + glm::vec3(0, -1, 0));
    if (it != chunks_.end())
        it->second->setIsChanged(true);

    // +z neighbor.
    it = chunks_.find(chunkPos + glm::vec3(0, 0, 1));
    if (it != chunks_.end())
        it->second->setIsChanged(true);

    // -z neighbor.
    it = chunks_.find(chunkPos + glm::vec3(0, 0, -1));
    if (it != chunks_.end())
        it->second->setIsChanged(true);

}

void chunkManager::pushDrawableChunks(const chunkRenderingData& renderingData)
{

    unique_lock<recursive_mutex> lock(drawableChunksWriteMutex_);

    drawableChunksWrite_->push_back(renderingData);

}

void chunkManager::swapDrawableChunksLists() 
{

    deque<chunkRenderingData>* aux = drawableChunksRead_;

    drawableChunksRead_ = drawableChunksWrite_;
    drawableChunksWrite_ = aux;

}

void chunkManager::loadChunk(const glm::vec3& chunkPos)
{

    bool chunkDoesntExist;

    {
    
        unique_lock<recursive_mutex> lock(chunksMutex_);

        chunkDoesntExist = chunks_.find(chunkPos) == chunks_.end();
    
    }

    if (chunkDoesntExist)
    {

        chunk* chunkPtr = nullptr;

        if (freeChunks_.size())
        {

            unique_lock<recursive_mutex> lock(freeChunksMutex_);

            chunkPtr = freeChunks_.front();
            freeChunks_.pop_front();

        }
        else
            chunkPtr = new chunk(*this);


        // TODO.
        // TEST IF THIS IS A CRITICAL SECTION.
        // Write chunk data section starts???

        chunkPtr->setIsChanged(true);
        chunkPtr->chunkPos() = chunkPos;

        // Terrain generation/loading.
        for (GLbyte x = 0; x < SCX; x++)
            for (GLbyte y = 0; y < SCY; y++)
                for (GLbyte z = 0; z < SCZ; z++)
                    chunkPtr->setBlock(x, y, z, (chunkPos.y <= 8) * (rand() % 2 + 1));

        // Write chunk data section ends???


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

                        // First check if there is any high priority chunk update.
                        // If so, force synchronization with the
                        // rendering thread to reflect the change made.
                        {

                            unique_lock<recursive_mutex> priorityListLock(highPriorityListMutex_);
                            if (!highPriorityList_.empty())
                            {

                                selectedChunk = selectChunkByChunkPos(highPriorityList_.front());
                                highPriorityList_.pop_front();
                                forceSyncFlag_ = true;

                            }
                            else
                                selectedChunk = nullptr;

                        }

                        // If a high priority update was found, process it and synchronize
                        // to reflect the change.
                        if (selectedChunk) 
                        {

                            // Unmark as freeable.
                            freeableChunksMutex_.lock();
                            freeableChunks_.erase(selectedChunk->chunkPos());
                            freeableChunksMutex_.unlock();

                            // Regenerate mesh and push for rendering if necessary. 
                            if (selectedChunk->getNBlocks())
                            {

                                selectedChunk->setIsChanged(false);
                                selectedChunk->renewMesh();

                                pushDrawableChunks(selectedChunk->renderingData());

                            }

                            // Sync with the other meshing threads and the chunk management thread.
                            syncPoint.arrive_and_wait();
                            meshingTsCVContinue = false;

                            // Wait for the chunk management thread's signal.
                            while (!meshingTsCVContinue)
                                meshingThreadsCV.wait(syncLock);

                        }

                        // Now continue processing common chunk updates.
                        { 
                        
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

                                    if (selectedChunk->getIsChanged())
                                    {

                                        selectedChunk->setIsChanged(false);
                                        selectedChunk->renewMesh();

                                    }

                                    pushDrawableChunks(selectedChunk->renderingData());

                                }

                            }
                            else
                                loadChunk(viewedChunkCoord);
                        
                        }

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

            // Reset the queue from previous iteration.
            drawableChunksWrite()->clear();

            // Marks all chunks as freeable
            for (unordered_map<glm::vec3, chunk*>::const_iterator it = chunks().cbegin(); it != chunks().cend(); it++)
                freeableChunks_.insert(it->first);


            // Signal all meshing threads to begin.
            meshingTsCVContinue = true;
            meshingThreadsCV.notify_all();

            // Wait for all meshing threads to finish.
            syncPoint.arrive_and_wait();

            // All remaining chunks marked as freeable are freed.
            // No thread can access the chunks dictionary  
            // at the same time this is being executed.
            chunksMutex_.lock();
            for (unordered_set<glm::vec3>::iterator it = freeableChunks_.begin(); it != freeableChunks_.end(); it++)
                unloadChunk(*it);
            chunksMutex_.unlock();

            // Reset the freeable chunks queue for the next iteration.
            freeableChunks_.clear();

            // Sync with the rendering thread.
            managerThreadCV_.wait(lock);


            // Here we prepare some things for the next iteration.

            // Increase chunk viewing range for the next
            // iteration until we reach the limit established
            // by the player's configuration.
            if (chunkRange <= nChunksToDraw())
                chunkRange++;

            // If a forcible synchronization was issued, reset
            // the corresponding flag.
            forceSyncFlag_ = false;

        }

        // Finish procedure.
        // First unblock any meshing thread so they can finish their execution properly.
        syncPoint.arrive_and_drop();
        meshingTsCVContinue = true;
        meshingThreadsCV.notify_all();
        // Wait for all meshing threads to end.
        for (int i = 0; i < nMeshingThreads; i++)
            meshingThreads[i].join();

    }

}

void chunkManager::highPriorityUpdate(const glm::vec3& chunkPos)
{

    chunk* selectedChunk = selectChunkByChunkPos(chunkPos);


    unique_lock<recursive_mutex> lock(highPriorityListMutex_);

    if (selectedChunk)
    {

        highPriorityList_.push_back(chunkPos);

    }

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