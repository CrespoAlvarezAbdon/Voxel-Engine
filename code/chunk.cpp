#include <cstring>
#include <cmath>
#include "chunk.h"
#include <iostream>
#include <ostream>

Chunk::Chunk(chunkManager& chunkManager)
    : changed_(true), nBlocks_(0), chunkManager_(chunkManager)
{

    renderingData_.chunkPos = glm::vec3(0, 0, 0);

    for (GLbyte x = 0; x < SCX; x++)
        for (GLbyte y = 0; y < SCY; y++)
            for (GLbyte z = 0; z < SCZ; z++)
                blocks_[x][y][z] = 0;

}

void Chunk::set(GLbyte x, GLbyte y, GLbyte z, Cube block_id)
{


    changed_ = true;

    if (blocks_[x][y][z] == 0 && block_id != 0)
        nBlocks_++;
    else
        if (blocks_[x][y][z] != 0 && block_id == 0 && nBlocks_ != 0)
            nBlocks_--;

    blocks_[x][y][z] = block_id;

}

void Chunk::renewMesh()
{

    // TODO. MOVE THESE VARIABLES TO AN APPROPIATE PLACE
    float atlasWidth = 512.0f,
          atlasHeight = 512.0f,
          textureWidth = 16.0f,
          textureHeight = 16.0f;

    renderingData_.vertices.clear();
    if (nBlocks_)
    {

        float texCoordX,
              texCoordY,
              texCoordX2,
              texCoordY2;

        // Get information about neighbor chunks.
        Chunk* frontNeighbor = chunkManager_.chunk(renderingData_.chunkPos.x, renderingData_.chunkPos.y, renderingData_.chunkPos.z + 1),
             * backNeighbor = chunkManager_.chunk(renderingData_.chunkPos.x, renderingData_.chunkPos.y, renderingData_.chunkPos.z - 1),
             * topNeighbor = chunkManager_.chunk(renderingData_.chunkPos.x, renderingData_.chunkPos.y + 1, renderingData_.chunkPos.z),
             * bottomNeighbor = chunkManager_.chunk(renderingData_.chunkPos.x, renderingData_.chunkPos.y - 1, renderingData_.chunkPos.z),
             * rightNeighbor = chunkManager_.chunk(renderingData_.chunkPos.x + 1, renderingData_.chunkPos.y, renderingData_.chunkPos.z),
             * leftNeighbor = chunkManager_.chunk(renderingData_.chunkPos.x - 1, renderingData_.chunkPos.y, renderingData_.chunkPos.z);

        for (GLbyte x = 0; x < SCX; x++)
            for (GLbyte y = 0; y < SCY; y++)
                for (GLbyte z = 0; z < SCZ; z++)
                {


                    // block_id = 0 means empty block or no block, so it doesn't have any vertices.
                    if (blocks_[x][y][z])
                    {

                        GLbyte xPlus1 = x + 1,
                                yPlus1 = y + 1,
                                zPlus1 = z + 1;

                        texCoordX = blocks_[x][y][z] / (atlasWidth / textureWidth);
                        texCoordY = (ceil(blocks_[x][y][z] / (atlasWidth / textureWidth))) / (atlasHeight / textureHeight);
                        texCoordX2 = texCoordX - 1 / ((atlasWidth / textureWidth));
                        texCoordY2 = texCoordY - 1 / ((atlasWidth / textureWidth));

                        // Hay que añadir una tercera condicion, que es exclusiva para los bloques que se hallan en el borde de un chunk.
                        // Esta condición consiste en que si no hay chunk vecino con el cual comparar, y la cara del chunk donde esta el bloque
                        // del que estamos hablando no es una cara límite (es decir, es un chunk que está al borde del rango de visión y la cara
                        // en cuestión sería la cara que roce el borde del rango de visión prácticamente), entonces dibujar la cara del bloque
                        // en cuestión.

                        // TODO. CHANGE 15 TO SCZ IN Z, TO SCY IN Y AND TO SCX IN X
                        // TODO. CHANGE THE LAST CONDITION (SIMILAR TO chunkManager_.chunk(chunkPos_.x, chunkPos_.y, chunkPos_.z + 1)->blocks_[x][y][0]) FOR EXAMPLE)
                        // TO A METHOD FOR CHECKING IF THE SAID BLOCK IS TRANSPARENT (THAT IS, IF IT BLOCKS THE VIEW OF THE FACES OF THE NEIGHBOR NON-AIR BLOCKS).
                        // TODO. get() method from Chunk class must not be called by two or more threads for the same Chunk object.
                        // TODO. Finish adding the right texture coordinates to all the faces.

                        // Vertices' generation.
                        // We don't want to render the faces of blocks at the boundary of the z and x axis if the neighbor block belongs to an unloaded chunk or it isn't solid because
                        // the player is never going to see those faces because, when the player tries to move there to see those faces, the neighbor chunks will be loaded. This has two possible
                        // outcomes. The first one is that the faces are rendered (because the neighboring block is transparent, like a glass block, for example) or that they aren't rendered
                        // because the neighbor block is solid and hides this face from the player.

                        // Front face vertices with culling of non-visible faces
                        if ((z < 15 && !blocks_[x][y][z + 1]) || (z == 15 && frontNeighbor && !frontNeighbor->get(x, y, 0)))
                        {

                            renderingData_.vertices.push_back(Block_vertex{ x,     y,      zPlus1, 0, texCoordX , texCoordY });
                            renderingData_.vertices.push_back(Block_vertex{ xPlus1, y,      zPlus1, 0, texCoordX2, texCoordY });
                            renderingData_.vertices.push_back(Block_vertex{ xPlus1, yPlus1, zPlus1, 0, texCoordX2, texCoordY2 });
                            renderingData_.vertices.push_back(Block_vertex{ xPlus1, yPlus1, zPlus1, 0, texCoordX2, texCoordY2 });
                            renderingData_.vertices.push_back(Block_vertex{ x,     yPlus1,  zPlus1, 0, texCoordX , texCoordY2 });
                            renderingData_.vertices.push_back(Block_vertex{ x,     y,     zPlus1, 0, texCoordX , texCoordY });

                        }

                        // Back face vertices with culling of non-visible faces

                        if ((z > 0 && !blocks_[x][y][z - 1]) || (z == 0 && backNeighbor && !backNeighbor->get(x, y, 15)))
                        {

                            renderingData_.vertices.push_back(Block_vertex{ xPlus1 , y,      z      , 0, texCoordX , texCoordY });
                            renderingData_.vertices.push_back(Block_vertex{ x      , y,      z      , 0, texCoordX2, texCoordY });
                            renderingData_.vertices.push_back(Block_vertex{ x      , yPlus1, z      , 0, texCoordX2, texCoordY2 });
                            renderingData_.vertices.push_back(Block_vertex{ x      , yPlus1, z      , 0, texCoordX2, texCoordY2 });
                            renderingData_.vertices.push_back(Block_vertex{ xPlus1 , yPlus1, z      , 0, texCoordX , texCoordY2 });
                            renderingData_.vertices.push_back(Block_vertex{ xPlus1 , y,      z      , 0, texCoordX , texCoordY });

                        }

                        // Top face vertices with culling of non-visible faces
                        if ((y < 15 && !blocks_[x][y + 1][z]) || (y == 15 && topNeighbor && !topNeighbor->get(x, 0, z)))
                        {

                            renderingData_.vertices.push_back(Block_vertex{ x      , yPlus1, zPlus1 , 0, texCoordX , texCoordY  });
                            renderingData_.vertices.push_back(Block_vertex{ xPlus1 , yPlus1, zPlus1 , 0, texCoordX2, texCoordY  });
                            renderingData_.vertices.push_back(Block_vertex{ xPlus1 , yPlus1, z      , 0, texCoordX2, texCoordY2 });
                            renderingData_.vertices.push_back(Block_vertex{ xPlus1 , yPlus1, z      , 0, texCoordX2, texCoordY2 });
                            renderingData_.vertices.push_back(Block_vertex{ x      , yPlus1, z      , 0, texCoordX , texCoordY2 });
                            renderingData_.vertices.push_back(Block_vertex{ x      , yPlus1, zPlus1 , 0, texCoordX , texCoordY  });

                        }

                        // Bottom face vertices with culling of non-visible faces
                        if ((y > 0 && !blocks_[x][y - 1][z]) || (y == 0 && bottomNeighbor && !bottomNeighbor->get(x, 0, z)))
                        {

                            renderingData_.vertices.push_back(Block_vertex{ x     , y      , z,      0, texCoordX , texCoordY });
                            renderingData_.vertices.push_back(Block_vertex{ xPlus1, y      , z,      0, texCoordX2, texCoordY });
                            renderingData_.vertices.push_back(Block_vertex{ xPlus1, y      , zPlus1, 0, texCoordX2, texCoordY2 });
                            renderingData_.vertices.push_back(Block_vertex{ xPlus1, y      , zPlus1, 0, texCoordX2, texCoordY2 });
                            renderingData_.vertices.push_back(Block_vertex{ x     , y      , zPlus1, 0, texCoordX , texCoordY2 });
                            renderingData_.vertices.push_back(Block_vertex{ x     , y      , z,      0, texCoordX , texCoordY });

                        }

                        // Right face vertices with culling of non-visible faces
                        if ((x < 15 && !blocks_[x + 1][y][z]) || (x == 15 && rightNeighbor && !rightNeighbor->get(0, y, z)))
                        {

                            renderingData_.vertices.push_back(Block_vertex{ xPlus1 , y     , zPlus1, 0, texCoordX , texCoordY });
                            renderingData_.vertices.push_back(Block_vertex{ xPlus1 , y     , z     , 0, texCoordX2, texCoordY });
                            renderingData_.vertices.push_back(Block_vertex{ xPlus1 , yPlus1, z     , 0, texCoordX2, texCoordY2 });
                            renderingData_.vertices.push_back(Block_vertex{ xPlus1 , yPlus1, z     , 0, texCoordX2, texCoordY2 });
                            renderingData_.vertices.push_back(Block_vertex{ xPlus1 , yPlus1, zPlus1, 0, texCoordX , texCoordY2 });
                            renderingData_.vertices.push_back(Block_vertex{ xPlus1 , y     , zPlus1, 0, texCoordX , texCoordY });

                        }

                        // Left face vertices with culling of non-visible faces
                        if ((x > 0 && !blocks_[x - 1][y][z]) || (x == 0 && leftNeighbor && !leftNeighbor->get(15, y, z)))
                        {

                            renderingData_.vertices.push_back(Block_vertex{ x     , y     , z     ,  0, texCoordX , texCoordY });
                            renderingData_.vertices.push_back(Block_vertex{ x     , y     , zPlus1,  0, texCoordX2, texCoordY });
                            renderingData_.vertices.push_back(Block_vertex{ x     , yPlus1, zPlus1,  0, texCoordX2, texCoordY2 });
                            renderingData_.vertices.push_back(Block_vertex{ x     , yPlus1, zPlus1,  0, texCoordX2, texCoordY2 });
                            renderingData_.vertices.push_back(Block_vertex{ x     , yPlus1, z     ,  0, texCoordX , texCoordY2 });
                            renderingData_.vertices.push_back(Block_vertex{ x     , y     , z     ,  0, texCoordX , texCoordY });

                        }

                    }

                }

    }

    changed_ = false;

}

Chunk::~Chunk()
{}

chunkManager::chunkManager(int nChunksToDraw, const Camera& playerCamera)
    : nChunksToDraw_(nChunksToDraw), playerCamera_(playerCamera), 
    drawableChunksRead_(new deque<chunkRenderingData>), drawableChunksWrite_(new deque<chunkRenderingData>)
{}

Chunk* chunkManager::chunk(GLbyte x, GLbyte y, GLbyte z)
{

    glm::vec3 chunkPos(x, y, z);

    unique_lock<recursive_mutex> lock(chunksMutex_);

    if (chunks_.find(chunkPos) != chunks_.end())
        return chunks_.at(chunkPos);
    else
        return nullptr;

}

Chunk* chunkManager::chunk(const glm::vec3& chunkPos)
{

    unique_lock<recursive_mutex> lock(chunksMutex_);

    if (chunks_.find(chunkPos) != chunks_.end())
        return chunks_.at(chunkPos);
    else
        return nullptr; 

}

void chunkManager::forceNeighborsToUpdate(const glm::vec3& chunkPos)
{

    unordered_map<glm::vec3, Chunk*>::iterator it;

    // +x neighbor
    it = chunks_.find(chunkPos + glm::vec3(1, 0, 0));
    if (it != chunks_.end())
        it->second->changed() = true;

    // -x neighbor
    it = chunks_.find(chunkPos + glm::vec3(-1, 0, 0));
    if (it != chunks_.end())
        it->second->changed() = true;

    // +y neighbor
    it = chunks_.find(chunkPos + glm::vec3(0, 1, 0));
    if (it != chunks_.end())
        it->second->changed() = true;

    // -y neighbor
    it = chunks_.find(chunkPos + glm::vec3(0, -1, 0));
    if (it != chunks_.end())
        it->second->changed() = true;

    // +z neighbor
    it = chunks_.find(chunkPos + glm::vec3(0, 0, 1));
    if (it != chunks_.end())
        it->second->changed() = true;

    // -z neighbor
    it = chunks_.find(chunkPos + glm::vec3(0, 0, -1));
    if (it != chunks_.end())
        it->second->changed() = true;

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

        Chunk* chunk = nullptr;

        if (freeChunks_.size())
        {

            unique_lock<recursive_mutex> lock(freeChunksMutex_);

            chunk = freeChunks_.front();
            freeChunks_.pop_front();

        }
        else
            chunk = new Chunk(*this);

        chunk->changed() = true;
        chunk->chunkPos() = chunkPos;

        // Terrain generation/loading.
        // Prepare the chunk's terrain before making it available to the chunk management threads.
        for (GLbyte x = 0; x < SCX; x++)
            for (GLbyte y = 0; y < SCY; y++)
                for (GLbyte z = 0; z < SCZ; z++)
                    chunk->set(x, y, z, (chunkPos.y <= 8) * (rand() % 2 + 1));

        // When this instruction is finished, the chunk will be available to all other chunk manager threads.
        {

            unique_lock<recursive_mutex> lock(chunksMutex_);

            chunks_.insert_or_assign(chunkPos, chunk);

        }

    }
     
}

void chunkManager::unloadChunk(const glm::vec3& chunkPos)
{

    unordered_map<glm::vec3, Chunk*>::iterator it;

    
    if ((it = chunks_.find(chunkPos)) != chunks_.end())
    {

        Chunk* unloadedChunk = it->second;
        chunks_.erase(chunkPos);
        freeChunks_.push_back(unloadedChunk);
    
    }

}

void chunkManager::meshChunks(const atomic<bool>& app_finished, const atomic<int>& chunkRange,
                              int rangeStart, int rangeEnd,
                              unordered_set<glm::vec3>& freeableChunks, mutex& freeableChunksMutex,
                              shared_mutex& syncMutex, condition_variable_any& meshingThreadsCV,
                              atomic<bool>& meshingTsCVContinue, barrier<>& syncPoint)
{

    {

        shared_lock syncLock(syncMutex);

        glm::vec3 playerChunkCoord,
            viewedChunkCoord,
            offset;
        Chunk* viewedChunk = nullptr;


        while (!app_finished)
        {

            // Wait for the chunk management thread's signal.
            while (!meshingTsCVContinue)
                meshingThreadsCV.wait(syncLock);

            cout << "[MESHING]: stopped waiting" << endl;

            playerChunkCoord = playerCamera_.chunkPos();

            for (offset.y = rangeStart; offset.y <= rangeEnd; offset.y++)
                for (offset.x = -chunkRange; offset.x <= chunkRange; offset.x++)
                    for (offset.z = -chunkRange; offset.z <= chunkRange; offset.z++)
                    {

                        viewedChunkCoord = playerChunkCoord + offset;
                        viewedChunk = chunk(viewedChunkCoord);

                        if (viewedChunk)
                        {

                            // Unmark as freeable those chunks that stay withing view range.
                            freeableChunksMutex.lock();
                            freeableChunks.erase(viewedChunkCoord);
                            freeableChunksMutex.unlock();

                            if (viewedChunk->hasBlocks())
                            {

                                if (viewedChunk->changed())
                                {

                                    viewedChunk->changed() = false;
                                    viewedChunk->renewMesh();

                                }

                                pushDrawableChunks(viewedChunk->renderingData());

                            }

                        }
                        else
                            loadChunk(viewedChunkCoord);

                    }

            // Sync with all other meshing threads and the chunk management thread.
            cout << "[MESHING THREAD]: waiting at barrier..." << endl;
            syncPoint.arrive_and_wait();
            meshingTsCVContinue = false;
            cout << "[MESHING THREAD]: resuming from barrier" << endl;

        }

    }

    // Finish procedure.
    syncPoint.arrive_and_drop();
    cout << "[MESHING THREAD]: finished!" << endl;


}

void chunkManager::manageChunks(const atomic<bool>& app_finished, unsigned int nMeshingThreads,
                                mutex& managerThreadMutex, condition_variable& managerThreadCV)
{

    vector<thread> meshingThreads;

    {

        unique_lock<mutex> lock(managerThreadMutex);

        atomic<int> chunkRange = 0;
        atomic<bool> meshingTsCVContinue = false;
        int rangeStart,
            rangeEnd,
            totalYChunks = Y_CHUNKS_RANGE * 2 + 1; // The total number of chunks rendered in the Y-axis.
        unordered_set<glm::vec3> freeableChunks;
        mutex freeableChunksMutex;
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
                                            ref(freeableChunks), ref(freeableChunksMutex),
                                            ref(syncMutex), ref(meshingThreadsCV),
                                            ref(meshingTsCVContinue), ref(syncPoint)));

        }

        // Chunk management main loop.
        while (!app_finished)
        {

            // Reset the queue from previous iteration.
            drawableChunksWrite()->clear();

            // Marks all chunks as freeable
            for (unordered_map<glm::vec3, Chunk*>::const_iterator it = chunks().cbegin(); it != chunks().cend(); it++)
                freeableChunks.insert(it->first);

            /*
            Meshing threads do their job and sync properly between themselves and
            the chunk management thread.
            */
            double time = glfwGetTime();

            // Signal all meshing threads to begin.
            meshingTsCVContinue = true;
            meshingThreadsCV.notify_all();

            // Wait for all meshing threads to finish.
            cout << "[MANAGEMENT]: waiting at barrier..." << endl;
            syncPoint.arrive_and_wait();

            cout << glfwGetTime() - time << endl;


            // All remaining chunks marked as freeable are freed.
            for (unordered_set<glm::vec3>::iterator it = freeableChunks.begin(); it != freeableChunks.end(); it++)
                unloadChunk(*it);

            // Reset the freeable chunks queue for the next iteration.
            freeableChunks.clear();

            // Sync with the rendering thread.
            managerThreadCV.wait(lock);

            // Load closest chunks first, then load those who are a little further and so on until 
            // the chunk rendering's maximun range is hit.
            if (chunkRange <= nChunksToDraw())
                chunkRange++;

        }

        // Finish procedure.
        // First unblock any meshing thread so they can finish their execution properly.
        cout << "[MANAGEMENT]: Waiting for joining..." << endl;
        syncPoint.arrive_and_drop();
        meshingTsCVContinue = true;
        meshingThreadsCV.notify_all();
        // Wait for all meshing threads to end.
        for (int i = 0; i < nMeshingThreads; i++)
            meshingThreads[i].join();

    }

    cout << "[DEBUG]: Chunk management thread ended" << endl;

}

chunkManager::~chunkManager()
{

    for (unordered_map<glm::vec3, Chunk*>::iterator it = chunks_.begin(); it != chunks_.end(); it++)
        if (it->second)
            delete it->second;

    for (deque<Chunk*>::iterator it = freeChunks_.begin(); it != freeChunks_.end(); it++)
        if (*it)
            delete *it;

    delete drawableChunksRead_;
    delete drawableChunksWrite_;

}