#include "chunk.h"
#include <iostream>
#include <fstream>
#include <format>
#include <chrono>
#include <filesystem>
#include <ctime>
#include <cstdlib>
#include <cmath>
#include <cstddef>
#include "model.h"
#include "gui.h"
#include "logger.h"
#include "timer.h"
#include "aiAPI.h"
#include "game.h"


namespace VoxelEng {

    ///////////////////////////////
    // Forward class declaration.//
    ///////////////////////////////

    class game;


    ////////////
    //Classes.//
    ////////////

    // 'chunk' class.

    bool chunk::initialised_ = false;
    const model* chunk::blockVertices_ = nullptr;
    const modelTriangles* chunk::blockTriangles_ = nullptr;


    void chunk::init() {
    
        if (initialised_)
            logger::errorLog("Chunk class's static member are already initialised");
        else {
        
            blockVertices_ = &models::getModelAt(1);
            blockTriangles_ = &models::getModelTriangles(1);

            initialised_ = true;
        
        }

    }

    chunk::chunk(bool empty, const vec3& chunkPos)
    : nBlocks_(0) {

        renderingData_.chunkPos = chunkPos;

        if (empty) {
        
            std::unique_lock<std::shared_mutex> lock(blocksMutex_);

            for (GLbyte x = 0; x < SCX; x++)
                for (GLbyte y = 0; y < SCY; y++)
                    for (GLbyte z = 0; z < SCZ; z++)
                        blocks_[x][y][z] = 0;
        
        }
        else
            worldGen::generate(*this); // This can only call to setBlock to modify the chunk and that method already takes care of 'blocksMutex_'.
        
    }

    chunk::chunk(const chunk& chunk)
    : nBlocks_(chunk.nBlocks_.load()) {

        std::unique_lock<std::shared_mutex> lock(blocksMutex_);

        for (GLbyte x = 0; x < SCX; x++)
            for (GLbyte y = 0; y < SCY; y++)
                for (GLbyte z = 0; z < SCZ; z++)
                    blocks_[x][y][z] = chunk.blocks_[x][y][z];

        renderingData_.chunkPos = chunk.renderingData_.chunkPos;

    }

    block chunk::getBlock(GLbyte x, GLbyte y, GLbyte z) {

        std::unique_lock<std::shared_mutex> lock(blocksMutex_);

        return blocks_[x][y][z];

    }

    block chunk::getBlock(const vec3& pos) {

        std::unique_lock<std::shared_mutex> lock(blocksMutex_);

        return blocks_[(unsigned int)pos.x][(unsigned int)pos.y][(unsigned int)pos.z];

    }

    block chunk::setBlock(const vec3& chunkRelPos, block blockID) {

        int x = chunkRelPos.x,
            y = chunkRelPos.y,
            z = chunkRelPos.z;

        std::unique_lock<std::shared_mutex> lock(blocksMutex_);

        changed_ = true;

        if (blocks_[x][y][z] == 0 && blockID != 0)
            nBlocks_++;
        else
            if (blocks_[x][y][z] != 0 && blockID == 0 && nBlocks_ != 0)
                nBlocks_--;

        block oldID = blocks_[x][y][z];
        blocks_[x][y][z] = blockID;
        return oldID;

    }

    block chunk::setBlock(GLbyte x, GLbyte y, GLbyte z, block blockID) {

        std::unique_lock<std::shared_mutex> lock(blocksMutex_);

        changed_ = true;

        if (blocks_[x][y][z] == 0 && blockID != 0)
            nBlocks_++;
        else
            if (blocks_[x][y][z] != 0 && blockID == 0 && nBlocks_ != 0)
                nBlocks_--;

        block oldID = blocks_[x][y][z];
        blocks_[x][y][z] = blockID;
        return oldID;

    }

    block chunk::setBlock(unsigned int linearIndex, block blockID) {

        GLbyte x = linearIndex / (SCZ * SCY),
               y = (linearIndex / SCZ) % SCY,
               z = linearIndex % SCZ;

        std::unique_lock<std::shared_mutex> lock(blocksMutex_);

        changed_ = true;

        if (blocks_[x][y][z] == 0 && blockID != 0)
            nBlocks_++;
        else
            if (blocks_[x][y][z] != 0 && blockID == 0 && nBlocks_ != 0)
                nBlocks_--;

        block oldID = blocks_[x][y][z];
        blocks_[x][y][z] = blockID;
        return oldID;

    }

    void chunk::renewMesh() {

        renderingData_.vertices.clear();
        if (nBlocks_) {

            block blockID = 0;


            // Get information about neighbor chunks.
            std::vector<chunk*> neighborChunks = {

                chunkManager::selectChunk(renderingData_.chunkPos.x, renderingData_.chunkPos.y, renderingData_.chunkPos.z + 1), // front
                chunkManager::selectChunk(renderingData_.chunkPos.x, renderingData_.chunkPos.y, renderingData_.chunkPos.z - 1), // back
                chunkManager::selectChunk(renderingData_.chunkPos.x, renderingData_.chunkPos.y + 1, renderingData_.chunkPos.z), // top
                chunkManager::selectChunk(renderingData_.chunkPos.x, renderingData_.chunkPos.y - 1, renderingData_.chunkPos.z), // bottom
                chunkManager::selectChunk(renderingData_.chunkPos.x + 1, renderingData_.chunkPos.y, renderingData_.chunkPos.z), // right
                chunkManager::selectChunk(renderingData_.chunkPos.x - 1, renderingData_.chunkPos.y, renderingData_.chunkPos.z) // left

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
            for (byte x = 0; x < SCX; x++)
                for (byte y = 0; y < SCY; y++)
                    for (byte z = 0; z < SCZ; z++) {

                        // Add block's model to the mesh if necessary.
                        if (blockID = blocks_[x][y][z]) {

                            bool DEBUG = blockID >= 7 && blockID <= 10;

                            // Front face vertices with culling of non-visible faces. z+
                            if (DEBUG || (z < 15 && !blocks_[x][y][z + 1]) || (z == 15 && neighborChunks[0] && !neighborChunks[0]->blocks_[x][y][0])) {

                                // Create the face's vertices.
                                for (int vertex = 0; vertex < blockTriangles_->operator[](0).size(); vertex++) {

                                    aux.positions[0] = renderingData_.chunkPos.x * SCX + x + blockVertices_->operator[](blockTriangles_->operator[](1)[vertex]).positions[0];
                                    aux.positions[1] = renderingData_.chunkPos.y * SCY + y + blockVertices_->operator[](blockTriangles_->operator[](1)[vertex]).positions[1];
                                    aux.positions[2] = renderingData_.chunkPos.z * SCZ + z + blockVertices_->operator[](blockTriangles_->operator[](1)[vertex]).positions[2];

                                    aux.normals = 0 | (0 << 30);
                                    aux.normals = aux.normals | (1023 << 20);
                                    aux.normals = aux.normals | (512 << 10);
                                    aux.normals = aux.normals | (512 << 0);

                                    renderingData_.vertices.push_back(aux);

                                }

                                // Add texture to the face.
                                models::addTexture(blockID, blockID, renderingData_.vertices);

                            }

                            // Back face vertices with culling of non-visible faces. z-
                            if (DEBUG || (z > 0 && !blocks_[x][y][z - 1]) || (z == 0 && neighborChunks[1] && !neighborChunks[1]->blocks_[x][y][15])) {

                                // Create the face's vertices
                                for (int vertex = 0; vertex < blockTriangles_->operator[](0).size(); vertex++) {

                                    aux.positions[0] = renderingData_.chunkPos.x * SCX + x + blockVertices_->operator[](blockTriangles_->operator[](0)[vertex]).positions[0];
                                    aux.positions[1] = renderingData_.chunkPos.y * SCY + y + blockVertices_->operator[](blockTriangles_->operator[](0)[vertex]).positions[1];
                                    aux.positions[2] = renderingData_.chunkPos.z * SCZ + z + blockVertices_->operator[](blockTriangles_->operator[](0)[vertex]).positions[2];

                                    aux.normals = 0 | (0 << 30);
                                    aux.normals = aux.normals | (0 << 20);
                                    aux.normals = aux.normals | (512 << 10);
                                    aux.normals = aux.normals | (512 << 0);

                                    renderingData_.vertices.push_back(aux);

                                }

                                // Add texture to the face.
                                models::addTexture(blockID, blockID, renderingData_.vertices);

                            }

                            // Top face vertices with culling of non-visible faces. y+
                            if (DEBUG || (y < 15 && !blocks_[x][y + 1][z]) || (y == 15 && neighborChunks[2] && !neighborChunks[2]->blocks_[x][0][z])) {

                                // Create the face's vertices
                                for (int vertex = 0; vertex < blockTriangles_->operator[](0).size(); vertex++) {

                                    aux.positions[0] = renderingData_.chunkPos.x * SCX + x + blockVertices_->operator[](blockTriangles_->operator[](2)[vertex]).positions[0];
                                    aux.positions[1] = renderingData_.chunkPos.y * SCY + y + blockVertices_->operator[](blockTriangles_->operator[](2)[vertex]).positions[1];
                                    aux.positions[2] = renderingData_.chunkPos.z * SCZ + z + blockVertices_->operator[](blockTriangles_->operator[](2)[vertex]).positions[2];

                                    aux.normals = 0 | (0 << 30);
                                    aux.normals = aux.normals | (512 << 20);
                                    aux.normals = aux.normals | (1023 << 10);
                                    aux.normals = aux.normals | (512 << 0);

                                    renderingData_.vertices.push_back(aux);

                                }

                                // Add texture to the face.
                                models::addTexture(blockID, blockID, renderingData_.vertices);

                            }

                            // Bottom face vertices with culling of non-visible faces. y-
                            if (DEBUG || (y > 0 && !blocks_[x][y - 1][z]) || (y == 0 && neighborChunks[3] && !neighborChunks[3]->blocks_[x][15][z])) {

                                // Create the face's vertices
                                for (int vertex = 0; vertex < blockTriangles_->operator[](0).size(); vertex++) {

                                    aux.positions[0] = renderingData_.chunkPos.x * SCX + x + blockVertices_->operator[](blockTriangles_->operator[](3)[vertex]).positions[0];
                                    aux.positions[1] = renderingData_.chunkPos.y * SCY + y + blockVertices_->operator[](blockTriangles_->operator[](3)[vertex]).positions[1];
                                    aux.positions[2] = renderingData_.chunkPos.z * SCZ + z + blockVertices_->operator[](blockTriangles_->operator[](3)[vertex]).positions[2];

                                    aux.normals = 0 | (0 << 30);
                                    aux.normals = aux.normals | (512 << 20);
                                    aux.normals = aux.normals | (0 << 10);
                                    aux.normals = aux.normals | (512 << 0);

                                    renderingData_.vertices.push_back(aux);

                                }

                                // Add texture to the face.
                                models::addTexture(blockID, blockID, renderingData_.vertices);

                            }

                            // Right face vertices with culling of non-visible faces. x+
                            if (DEBUG || (x < 15 && !blocks_[x + 1][y][z]) || (x == 15 && neighborChunks[4] && !neighborChunks[4]->blocks_[0][y][z])) {

                                // Create the face's vertices
                                for (int vertex = 0; vertex < blockTriangles_->operator[](0).size(); vertex++) {

                                    aux.positions[0] = renderingData_.chunkPos.x * SCX + x + blockVertices_->operator[](blockTriangles_->operator[](5)[vertex]).positions[0];
                                    aux.positions[1] = renderingData_.chunkPos.y * SCY + y + blockVertices_->operator[](blockTriangles_->operator[](5)[vertex]).positions[1];
                                    aux.positions[2] = renderingData_.chunkPos.z * SCZ + z + blockVertices_->operator[](blockTriangles_->operator[](5)[vertex]).positions[2];

                                    aux.normals = 0 | (0 << 30);
                                    aux.normals = aux.normals | (512 << 20);
                                    aux.normals = aux.normals | (512 << 10);
                                    aux.normals = aux.normals | (1023 << 0);

                                    renderingData_.vertices.push_back(aux);

                                }

                                // Add texture to the face.
                                models::addTexture(blockID, blockID, renderingData_.vertices);

                            }

                            // Left face vertices with culling of non-visible faces. x-
                            if (DEBUG || (x > 0 && !blocks_[x - 1][y][z]) || (x == 0 && neighborChunks[5] && !neighborChunks[5]->blocks_[15][y][z])) {

                                // Create the face's vertices
                                for (int vertex = 0; vertex < blockTriangles_->operator[](0).size(); vertex++) {

                                    aux.positions[0] = renderingData_.chunkPos.x * SCX + x + blockVertices_->operator[](blockTriangles_->operator[](4)[vertex]).positions[0];
                                    aux.positions[1] = renderingData_.chunkPos.y * SCY + y + blockVertices_->operator[](blockTriangles_->operator[](4)[vertex]).positions[1];
                                    aux.positions[2] = renderingData_.chunkPos.z * SCZ + z + blockVertices_->operator[](blockTriangles_->operator[](4)[vertex]).positions[2];

                                    aux.normals = 0 | (0 << 30);
                                    aux.normals = aux.normals | (512 << 20);
                                    aux.normals = aux.normals | (512 << 10);
                                    aux.normals = aux.normals | (0 << 0);

                                    renderingData_.vertices.push_back(aux);

                                }

                                // Add texture to the face.
                                models::addTexture(blockID, blockID, renderingData_.vertices);

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

    void chunk::makeEmpty() {
    
        std::unique_lock<std::shared_mutex> lock(blocksMutex_);

        changed_ = true;
        nBlocks_ = 0;

        for (GLbyte x = 0; x < SCX; x++)
            for (GLbyte y = 0; y < SCY; y++)
                for (GLbyte z = 0; z < SCZ; z++)
                    blocks_[x][y][z] = 0;

    }

    void chunk::cleanUp() {
    
        blockVertices_ = nullptr;
        blockTriangles_ = nullptr;

        initialised_ = false;
    
    }


    // 'chunkManager' class.

    bool chunkManager::initialised_ = false,
         chunkManager::infiniteWorld_ = false;
    int chunkManager::nChunksToCompute_ = 0;
    std::unordered_map<vec3, chunk*> chunkManager::chunks_;
    std::unordered_map<vec3, std::vector<vertex>>* chunkManager::drawableChunksWrite_ = nullptr,
                                                 * chunkManager::drawableChunksRead_ = nullptr;
    std::deque<chunk*> chunkManager::freeChunks_;
    std::unordered_set<vec3> chunkManager::freeableChunks_;

    std::deque<vec3> chunkManager::priorityMeshingList_;
    std::deque<vec3> chunkManager::priorityUpdateList_;

    std::mutex chunkManager::freeableChunksMutex_,
               chunkManager::managerThreadMutex_,
               chunkManager::loadingTerrainMutex_;
    std::shared_mutex chunkManager::highPriorityMutex_;
    std::recursive_mutex chunkManager::drawableChunksWriteMutex_,
                         chunkManager::chunksMutex_,
                         chunkManager::freeChunksMutex_,
                         chunkManager::priorityMeshingListMutex_;
    std::condition_variable chunkManager::managerThreadCV_,
                            chunkManager::loadingTerrainCV_;
    std::condition_variable_any chunkManager::highPriorityUpdatesCV_;

    std::atomic<bool> chunkManager::forceSyncFlag_ = false,
                      chunkManager::waitTerrainLoaded_ = true;

    unsigned int chunkManager::parseChunkPosState_ = 0;
    const unsigned int chunkManager::parseChunkPosStates_ = 2;
    std::string chunkManager::openedTerrainFileName_ = "";

    std::unordered_map<unsigned int, std::unordered_map<vec3, chunk*>> chunkManager::AIagentChunks_;
    unsigned int chunkManager::selectedAIWorld_ = 0;

    bool chunkManager::AImodeON_ = false,
         chunkManager::originalWorldAccess_ = true;


    void chunkManager::init(unsigned int nChunksToCompute) {

        if (initialised_)
            logger::errorLog("Chunk management system was already initialised");
        else {

            infiniteWorld_ = false;

            nChunksToCompute_ = nChunksToCompute;

            drawableChunksRead_ = new std::unordered_map<vec3, std::vector<vertex>>;
            drawableChunksWrite_ = new std::unordered_map<vec3, std::vector<vertex>>;

            parseChunkPosState_ = 0;
            selectedAIWorld_ = 0;

            forceSyncFlag_ = false;
            waitTerrainLoaded_ = true;

            openedTerrainFileName_ = "";

            AImodeON_ = false;
            originalWorldAccess_ = true;

            initialised_ = true;
        
        } 

    }

    vec3 chunkManager::getChunkRelCoords(float globalX, float globalY, float globalZ) {

        int floorX = std::floor(globalX),
            floorY = std::floor(globalY),
            floorZ = std::floor(globalZ);

        return vec3(floorMod((floorX >= 0) ? floorX : SCX + floorX, SCX),
                    floorMod((floorY >= 0) ? floorY : SCY + floorY, SCY),
                    floorMod((floorZ >= 0) ? floorZ : SCZ + floorZ, SCZ));

    }

    block chunkManager::getBlock(int posX, int posY, int posZ) {

        vec3 chunkPos = getChunkCoords(posX, posY, posZ);
        block selectedBlock;


        std::unique_lock<std::recursive_mutex> lock(chunksMutex_);

        if (AImodeON_) {
        
            if (originalWorldAccess_ || AIagentChunks_.find(selectedAIWorld_) == AIagentChunks_.cend() ||
                AIagentChunks_[selectedAIWorld_].find(chunkPos) == AIagentChunks_[selectedAIWorld_].cend()) { // Search in original's level.
            
                if (chunks_.find(chunkPos) == chunks_.end())
                    logger::errorLog("Chunk " + std::to_string(chunkPos.x) + "|" + std::to_string(chunkPos.y) + "|" + std::to_string(chunkPos.z) + " does not exist");
                else
                    selectedBlock = chunks_[chunkPos]->getBlock(floorMod(posX, SCX), (int)floor(posY) % SCY, floorMod(posZ, SCZ));
            
            }
            else // Search in agent's level.
                selectedBlock = AIagentChunks_[selectedAIWorld_][chunkPos]->getBlock(floorMod(posX, SCX), (int)floor(posY) % SCY, floorMod(posZ, SCZ));

        }
        else {
        
            if (chunks_.find(chunkPos) == chunks_.end())
                logger::errorLog("Chunk " + std::to_string(chunkPos.x) + "|" + std::to_string(chunkPos.y) + "|" + std::to_string(chunkPos.z) + " does not exist");
            else
                selectedBlock = chunks_[chunkPos]->getBlock(floorMod(posX, SCX), (int)floor(posY) % SCY, floorMod(posZ, SCZ));
            
        }

        return selectedBlock;

    }

    std::vector<block> chunkManager::getBlocksBox(int x1, int y1, int z1, int x2, int y2, int z2) {

        std::vector<block> blocks;

        if (x1 <= x2 && y1 <= y2 && z1 <= z2) {

            chunk* selectedChunk;

            std::unique_lock<std::recursive_mutex> lock(chunksMutex_);

            for (int i = x1; i <= x2; i++) // No merece la pena hacer cach� porque las posiciones de los bloques pueden estar puestas "a mala leche" y seguir dando un peor caso.
                for (int j = y1; j <= y2; j++)
                    for (int k = z1; k <= z2; k++)
                        blocks.push_back(getBlock((int)floor((double)i / SCX), (int)floor((double)j / SCY), (int)floor((double)k / SCZ)));

        }
        else
            logger::errorLog("pos1.x <= pos2.x && pos1.y <= pos2.y && pos1.z <= pos2.z is not satisfied when getting blocks with a selection box");

        return blocks;

    }

    chunkLoadLevel chunkManager::getChunkLoadLevel(const vec3& chunkPos) {

        std::unique_lock<std::recursive_mutex> lock(chunksMutex_);

        return (chunks_.find(chunkPos) != chunks_.cend() ? chunks_[chunkPos]->loadLevel() : chunkLoadLevel::NOTLOADED);

    }

    void chunkManager::setAImode(bool ON) {

        if (game::loopSelection() == GRAPHICALLEVEL)
            logger::errorLog("Cannot change chunkManager's AI mode while in a level");
        else
            AImodeON_ = ON;

    }

    void chunkManager::setNChunksToCompute(unsigned int nChunksToCompute) {
    
        // That is, if terrain has already been loaded, the number of chunks to compute
        // cannot be changed (for now).

        if (infiniteWorld_ || game::loopSelection() != GRAPHICALLEVEL)
            nChunksToCompute_ = nChunksToCompute;
        else
            logger::errorLog("Cannot change number of chunks to compute in a finite world that has been loaded.");
    
    }

    block chunkManager::setBlock(int x, int y, int z, block blockID) {
    
        vec3 chunkPos = getChunkCoords(x, y, z);


        std::unique_lock<std::recursive_mutex> lock(chunksMutex_);

        if (AImodeON_) {
        
            if (AIagentChunks_.find(selectedAIWorld_) == AIagentChunks_.cend()) // Insert an empty map and fill it with the 
                AIagentChunks_[selectedAIWorld_] = std::unordered_map<vec3, chunk*>(); // differences between the original level and the agent copy.

            std::unordered_map<vec3, chunk*>& agentWorld = AIagentChunks_[selectedAIWorld_];

            if (agentWorld.find(chunkPos) == agentWorld.cend()) {
            
                if (chunks_.find(chunkPos) == chunks_.cend())
                    agentWorld[chunkPos] = new chunk(*chunks_[chunkPos]);  
                else
                    logger::errorLog("There is no chunk " + std::to_string(chunkPos.x) + '|' + std::to_string(chunkPos.y) + '|' + std::to_string(chunkPos.z) + 
                                     "for AI agent " + std::to_string(selectedAIWorld_));
            
            }
            
            return agentWorld[chunkPos]->setBlock(getChunkRelCoords(x, y, z), blockID);
        
        }
        else {
        
            if (chunks_.find(chunkPos) == chunks_.cend())
                logger::errorLog("Chunk " + std::to_string(chunkPos.x) + "|" + std::to_string(chunkPos.y) + "|" + std::to_string(chunkPos.z) + " does not exist");
            else
                return chunks_[chunkPos]->setBlock(getChunkRelCoords(x, y, z), blockID);
                
        }

    }

    chunk* chunkManager::createChunkAt(bool empty, const vec3& chunkPos) {

        std::unique_lock<std::recursive_mutex> lock(chunksMutex_);

        if (chunks_.find(chunkPos) == chunks_.end())
            return createChunk(empty, chunkPos);
        else
            logger::errorLog("Chunk at " + std::to_string(chunkPos.x) + '|' + std::to_string(chunkPos.y) + '|' + std::to_string(chunkPos.z) + " already exists");

    }

    chunk* chunkManager::createChunk(bool empty, const vec3& chunkPos) {

        std::unique_lock<std::recursive_mutex> lock(chunksMutex_);

        chunk* selectedChunk = new chunk(false, chunkPos);
        chunks_.insert_or_assign(chunkPos, selectedChunk);
        return selectedChunk;

    }

    chunk* chunkManager::selectChunk(GLbyte x, GLbyte y, GLbyte z) {

        vec3 chunkPos(x, y, z);
        std::unique_lock<std::recursive_mutex> lock(chunksMutex_);


        if (chunks_.find(chunkPos) != chunks_.end())
            return chunks_.at(chunkPos);
        else
            return nullptr;

    }

    chunk* chunkManager::selectChunkByChunkPos(const vec3& chunkPos) {

        std::unique_lock<std::recursive_mutex> lock(chunksMutex_);


        if (chunks_.find(chunkPos) != chunks_.end())
            return chunks_.at(chunkPos);
        else
            return nullptr;

    }

    chunk* chunkManager::selectChunkByChunkPos(int x, int y, int z) {

        vec3 chunkPos = getChunkCoords(x, y, z);
        std::unique_lock<std::recursive_mutex> lock(chunksMutex_);


        if (chunks_.find(chunkPos) != chunks_.end())
            return chunks_.at(chunkPos);
        else
            return nullptr;

    }

    chunk* chunkManager::selectChunkByRealPos(const vec3& pos) {

        vec3 chunkPos = getChunkCoords(pos);


        std::unique_lock<std::recursive_mutex> lock(chunksMutex_);

        if (chunks_.find(chunkPos) != chunks_.end())
            return chunks_.at(chunkPos);
        else
            return nullptr;

    }

    chunk* chunkManager::neighborMinusX(const vec3& chunkPos) {

        vec3 neighborChunkPos(chunkPos.x - 1, chunkPos.y, chunkPos.z);


        std::unique_lock<std::recursive_mutex> lock(chunksMutex_);

        if (chunks_.find(neighborChunkPos) != chunks_.end())
            return chunks_.at(neighborChunkPos);
        else
            return nullptr;

    }

    chunk* chunkManager::neighborPlusX(const vec3& chunkPos) {

        vec3 neighborChunkPos(chunkPos.x + 1, chunkPos.y, chunkPos.z);


        std::unique_lock<std::recursive_mutex> lock(chunksMutex_);

        if (chunks_.find(neighborChunkPos) != chunks_.end())
            return chunks_.at(neighborChunkPos);
        else
            return nullptr;

    }

    chunk* chunkManager::neighborMinusY(const vec3& chunkPos) {

        vec3 neighborChunkPos(chunkPos.x, chunkPos.y - 1, chunkPos.z);


        std::unique_lock<std::recursive_mutex> lock(chunksMutex_);

        if (chunks_.find(neighborChunkPos) != chunks_.end())
            return chunks_.at(neighborChunkPos);
        else
            return nullptr;

    }

    chunk* chunkManager::neighborPlusY(const vec3& chunkPos) {

        vec3 neighborChunkPos(chunkPos.x, chunkPos.y + 1, chunkPos.z);


        std::unique_lock<std::recursive_mutex> lock(chunksMutex_);

        if (chunks_.find(neighborChunkPos) != chunks_.end())
            return chunks_.at(neighborChunkPos);
        else
            return nullptr;

    }

    chunk* chunkManager::neighborMinusZ(const vec3& chunkPos) {

        vec3 neighborChunkPos(chunkPos.x, chunkPos.y, chunkPos.z - 1);


        std::unique_lock<std::recursive_mutex> lock(chunksMutex_);

        if (chunks_.find(neighborChunkPos) != chunks_.end())
            return chunks_.at(neighborChunkPos);
        else
            return nullptr;

    }

    chunk* chunkManager::neighborPlusZ(const vec3& chunkPos) {

        vec3 neighborChunkPos(chunkPos.x, chunkPos.y, chunkPos.z + 1);


        std::unique_lock<std::recursive_mutex> lock(chunksMutex_);

        if (chunks_.find(neighborChunkPos) != chunks_.end())
            return chunks_.at(neighborChunkPos);
        else
            return nullptr;

    }

    void chunkManager::waitTerrainLoaded() {

        {

            std::unique_lock<std::mutex> lock(loadingTerrainMutex_);
            while(waitTerrainLoaded_)
                loadingTerrainCV_.wait(lock);

        }

    }

    void chunkManager::pushDrawableChunks(const chunkRenderingData& renderingData) {

        std::unique_lock<std::recursive_mutex> lock(drawableChunksWriteMutex_);


        drawableChunksWrite_->insert_or_assign(renderingData.chunkPos, renderingData.vertices);

    }

    void chunkManager::swapDrawableChunksLists() {

        std::unordered_map<vec3, std::vector<vertex>>* aux = drawableChunksRead_;


        drawableChunksRead_ = drawableChunksWrite_;
        drawableChunksWrite_ = aux;

    }

    void chunkManager::updatePriorityChunks() {

        vec3 chunkPos;
        while (!priorityUpdateList_.empty()) {

            chunkPos = priorityUpdateList_.front();

            priorityUpdateList_.pop_front();

            if (drawableChunksWrite_->contains(chunkPos))
                drawableChunksRead_->insert_or_assign(chunkPos, drawableChunksWrite_->at(chunkPos));

        }

    }

    void chunkManager::loadChunk(const vec3& chunkPos)
    {

        bool chunkNotLoaded;


        {

            std::unique_lock<std::recursive_mutex> lock(chunksMutex_);

            chunkNotLoaded = chunks_.find(chunkPos) == chunks_.end();

        }

        if (chunkNotLoaded)
        {

            chunk* chunkPtr = nullptr;

            {

                std::unique_lock<std::recursive_mutex> lock(freeChunksMutex_);

                if (freeChunks_.size())
                {

                    chunkPtr = freeChunks_.front();
                    freeChunks_.pop_front();

                }
                else
                    chunkPtr = new chunk(true);

            }

            chunkPtr->changed() = true;
            chunkPtr->chunkPos() = chunkPos;
            worldGen::generate(*chunkPtr);

            {

                std::unique_lock<std::recursive_mutex> lock(chunksMutex_);

                chunks_.insert_or_assign(chunkPos, chunkPtr);

            }

        }

    }

    void chunkManager::unloadChunk(const vec3& chunkPos) {

        std::unordered_map<vec3, chunk*>::iterator it;
        std::unique_lock<std::recursive_mutex> lock(chunksMutex_);
        std::unique_lock<std::recursive_mutex> lockFree(freeChunksMutex_);

        if ((it = chunks_.find(chunkPos)) != chunks_.end()) {

            chunk* unloadedChunk = it->second;
            chunks_.erase(chunkPos);
            freeChunks_.push_back(unloadedChunk);

        }

    }

    void chunkManager::meshChunks(const std::atomic<int>& chunkRange, int rangeStart, int rangeEnd,
                                  std::shared_mutex& syncMutex, std::condition_variable_any& meshingThreadsCV,
                                  std::atomic<bool>& meshingTsCVContinue, std::barrier<>& syncPoint) {

        {

            std::shared_lock syncLock(syncMutex);

            vec3 playerChunkCoord,
                 viewedChunkCoord,
                 offset;
            chunk* selectedChunk = nullptr;


            while (game::loopSelection() == GRAPHICALLEVEL) {

                // Wait for the chunk management thread's signal.
                while (!meshingTsCVContinue)
                    meshingThreadsCV.wait(syncLock);

                playerChunkCoord = camera::cPlayerCamera()->chunkPos();

                // World loading.
                for (offset.y = rangeStart; offset.y <= rangeEnd; offset.y++)
                    for (offset.x = -chunkRange; offset.x < chunkRange; offset.x++)
                        for (offset.z = -chunkRange; offset.z < chunkRange; offset.z++)
                        {

                            // First check if there is any high priority chunk update.
                            // If so, force synchronization with the
                            // rendering thread to reflect the change made.
                            {

                                std::unique_lock<std::recursive_mutex> priorityListLock(priorityMeshingListMutex_);
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

    void chunkManager::manageChunks(unsigned int nMeshingThreads) {

        std::vector<std::thread> meshingThreads;


        {

            std::unique_lock<std::mutex> lock(managerThreadMutex_);

            // The total number of chunks rendered in the Y-axis.
            // This value is different from the values in the
            // other axes to prevent slow chunk loading while
            // maintaining a good render distance range.
            int rangeStart,
                rangeEnd;
            std::atomic<int> chunkRange = 0;
            std::atomic<bool> meshingTsCVContinue = false;
            std::shared_mutex syncMutex;
            std::condition_variable_any meshingThreadsCV;
            std::barrier syncPoint(nMeshingThreads + 1);
            chunk* priorityChunk = nullptr;


            // Initialize meshing threads.
            for (int i = 0; i < nMeshingThreads; i++) {

                rangeStart = totalYChunks / nMeshingThreads * i;
                if (i == nMeshingThreads - 1)
                    rangeEnd = totalYChunks;
                else
                    rangeEnd = rangeStart + totalYChunks / nMeshingThreads - 1;

                meshingThreads.push_back(std::thread(&chunkManager::meshChunks,
                                         ref(chunkRange), rangeStart - yChunksRange, rangeEnd - yChunksRange,
                                         ref(syncMutex), ref(meshingThreadsCV),
                                         ref(meshingTsCVContinue), ref(syncPoint)));

            }

            // Chunk management main loop.
            while (game::loopSelection() == GRAPHICALLEVEL) {

                // This will only execute when a high priority chunk update
                // is not issued.
                if (!forceSyncFlag_)
                {

                    // Marks all chunks as freeable
                    for (std::unordered_map<vec3, chunk*>::const_iterator it = chunks().cbegin(); it != chunks().cend(); it++)
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
                    for (std::unordered_set<vec3>::iterator it = freeableChunks_.begin(); it != freeableChunks_.end(); it++)
                        unloadChunk(*it);
                    chunksMutex_.unlock();

                    // Increase chunk viewing range for the next
                    // iteration until we reach the limit established
                    // by the player's configuration.
                    if (chunkRange <= nChunksToCompute_)
                        chunkRange++;

                    // Sync with the rendering thread.
                    managerThreadCV_.wait(lock);

                    // Reset some data structures for the next iteration.
                    freeableChunks_.clear();
                    drawableChunksWrite_->clear();

                }
                else { // If there is a high priority chunk update, process it.

                    vec3 priorityChunkPos;
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

                    if (isLockActive)
                        priorityMeshingListMutex_.unlock();

                    // Sync with the rendering thread if necessary.
                    if (synchronize)
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

            drawableChunksRead_->clear();
            drawableChunksWrite_->clear();

        }

    }

    void chunkManager::finiteWorldLoading(const std::string& terrainFile) {

        chunk* selectedChunk = nullptr;
        vec3 chunkPos;


        {

            std::unique_lock<std::mutex> lock(managerThreadMutex_);

            /*
            World loading if necessary.
            */
            if (terrainFile.empty()) {
            
                std::unique_lock<std::recursive_mutex> lock(chunksMutex_);


                if (unsigned int slot = game::selectedSaveSlot())
                    loadAllChunks("saves/slot" + std::to_string(slot) + "/level");
                else {

                    worldGen::prepareGen();

                    for (chunkPos.y = -yChunksRange; chunkPos.y < yChunksRange; chunkPos.y++)
                        for (chunkPos.x = -nChunksToCompute_; chunkPos.x < nChunksToCompute_; chunkPos.x++)
                            for (chunkPos.z = -nChunksToCompute_; chunkPos.z < nChunksToCompute_; chunkPos.z++) {

                                selectedChunk = new chunk(false, chunkPos);
                                chunks_.insert_or_assign(chunkPos, selectedChunk);

                            }

                }
            
            }

            // Signal that the terrain loaded has finished.
            waitTerrainLoaded_ = false;
            loadingTerrainCV_.notify_all();

            // Once chunk data has been loaded, generate the meshes. All chunk data must be loaded first before generating any mesh to
            // compute block face culling optimizations.
            for (chunkPos.y = -yChunksRange; chunkPos.y < yChunksRange; chunkPos.y++)
                for (chunkPos.x = -nChunksToCompute_; chunkPos.x < nChunksToCompute_; chunkPos.x++)
                    for (chunkPos.z = -nChunksToCompute_; chunkPos.z < nChunksToCompute_; chunkPos.z++) {

                        selectedChunk = selectChunkByChunkPos(chunkPos);

                        selectedChunk->renewMesh();
                        selectedChunk->changed() = false;

                        if (selectedChunk->renderingData().vertices.size())
                            pushDrawableChunks(selectedChunk->renderingData());

                    }

            // Sync with rendering thread.
            managerThreadCV_.wait(lock);

            // Handle high priority chunk updates (the only type of chunk updates for
            // this type of world loading).
            while (game::loopSelection() == GRAPHICALLEVEL) {

                forceSyncFlag_ = !priorityMeshingList_.empty();

                if (forceSyncFlag_) {

                    vec3 priorityChunkPos;
                    bool synchronize = false,
                         isLockActive = false;


                    priorityMeshingListMutex_.lock();
                    isLockActive = true;

                    while (!priorityMeshingList_.empty()) {

                        priorityChunkPos = priorityMeshingList_.front();
                        priorityMeshingList_.pop_front();

                        priorityMeshingListMutex_.unlock();
                        isLockActive = false;

                        selectedChunk = selectChunkByChunkPos(priorityChunkPos);

                        if (selectedChunk) {

                            synchronize = true;

                            // Update mesh. 
                            selectedChunk->changed() = false;
                            selectedChunk->renewMesh();

                            pushDrawableChunks(selectedChunk->renderingData());
                            priorityUpdateList_.push_back(selectedChunk->chunkPos());

                        }

                        priorityMeshingListMutex_.lock();
                        isLockActive = true;

                    }

                    if (isLockActive)
                        priorityMeshingListMutex_.unlock();

                    // Sync with the rendering thread if necessary.
                    if (synchronize)
                        managerThreadCV_.wait(lock);

                    forceSyncFlag_ = false;

                }

            }

            drawableChunksRead_->clear();
            drawableChunksWrite_->clear();

        }

    }

    void chunkManager::generateAIWorld(const std::string& path) {

        if (AImodeON_) {

            chunk* selectedChunk = nullptr;
            vec3 chunkPos;


            if (path.empty()) {

                std::srand(std::time(nullptr)); // Initalize random generator for creating new level.

                for (chunkPos.y = -yChunksRange; chunkPos.y < yChunksRange; chunkPos.y++)
                    for (chunkPos.x = -nChunksToCompute_; chunkPos.x < nChunksToCompute_; chunkPos.x++)
                        for (chunkPos.z = -nChunksToCompute_; chunkPos.z < nChunksToCompute_; chunkPos.z++) {

                            selectedChunk = new chunk(false, chunkPos);
                            chunks_.insert_or_assign(chunkPos, selectedChunk);

                        }

            }
            else
                loadAllChunks(path);

        }
        else
            logger::errorLog("Chunk manager's AI mode must be turned on when generating a world for AI testing/training.");

    }

    void chunkManager::saveAllChunks(const std::string& path) {

        // Save chunk data.
        std::ofstream saveFile(path + ".terrain");
        std::string saveData,
                    nChunks = std::to_string(chunks_.size());

        // We cannot really be sure about the total size of the data to be saved as there can be blocks with IDs with 1, 2 or even more digits.
        // Try to guess the actual size of the data to be stored and reserve more main memory to the std::string buffer as it does not involve too much
        // expensive memory allocation.
        timer t;
        t.start();

        saveData += std::to_string(nChunksToCompute_) + '|';

        unsigned int nChunksSaved = 0;
        for (auto it = chunks_.begin(); it != chunks_.end(); it++) {

            if (it->second->getNBlocks()) {
            
                saveData += std::to_string((int)it->first.x) + '|';
                saveData += std::to_string((int)it->first.y) + '|';
                saveData += std::to_string((int)it->first.z) + "|-";

                for (int x = 0; x < SCX; x++)
                    for (int y = 0; y < SCY; y++)
                        for (int z = 0; z < SCZ; z++)
                            saveData += std::to_string(it->second->getBlock(x, y, z)) + '|';

                saveData += '-';

                nChunksSaved++;
            
            }

        }

        t.clean();
        logger::debugLog("Time: " + std::to_string(t.getDurationMs()) + " ms");

        t.start();
        saveFile << saveData;
        t.clean();
        logger::debugLog("Time: " + std::to_string(t.getDurationMs()) + " ms");

    }

    void chunkManager::loadAllChunks(const std::string& path) {

        std::string truePath = path + ".terrain";
        std::ifstream saveFile(truePath);
       
        
        if (std::filesystem::exists(truePath)) {

            openedTerrainFileName_ = path;

            std::string saveData,
                        word = "";
            vec3 chunkPos;
            chunk* selectedChunk = nullptr;


            // Read from disk into main memory.
            timer t;
            t.start();
            saveFile.seekg(0, std::ios::end);
            saveData.resize(saveFile.tellg());
            saveFile.seekg(0);
            saveFile.read(saveData.data(), saveData.size());
            saveFile.close();


            // Parse data in main memory.
            bool isChunkSelected = false;
            unsigned int chunkPosCoord = 0, // 0 -> x, 1 -> y, 2 -> z.
                         chunkLinearIndex = 0,
                         parseState = 0; // 0 = reading nChunksToCompute, 1 = reading a chunk's position and 2 = reading a chunk's block data.
            int number = 0;
            for (int i = 0; i < saveData.size(); i++) {

                if (saveData[i] == '-')
                    parseState = (parseState == 1) ? 2 : 1;
                else if (saveData[i] == '|') {

                    number = std::stoi(word);
                    word = "";

                    switch (parseState) {
                    
                        case 0:

                            nChunksToCompute_ = number;

                            // Load chunk structures.
                            for (chunkPos.y = -yChunksRange; chunkPos.y < yChunksRange; chunkPos.y++)
                                for (chunkPos.x = -nChunksToCompute_; chunkPos.x < nChunksToCompute_; chunkPos.x++)
                                    for (chunkPos.z = -nChunksToCompute_; chunkPos.z < nChunksToCompute_; chunkPos.z++) {

                                        selectedChunk = new chunk(true, chunkPos);
                                        chunks_.insert_or_assign(chunkPos, selectedChunk);

                                    }

                            parseState++;

                            break;

                        case 1:

                            chunkLinearIndex = 0;
                            isChunkSelected = false;

                            if (chunkPosCoord == 0)
                                chunkPos.x = number;
                            else if (chunkPosCoord == 1)
                                chunkPos.y = number;
                            else
                                chunkPos.z = number;

                            chunkPosCoord = (chunkPosCoord + 1) % 3; // The next number read (if not the third one read) will be stored
                                                                     // in the next chunk position coordinate.

                            break;

                        case 2:

                            if (!isChunkSelected) {

                                selectedChunk = selectChunkByChunkPos(chunkPos);
                                isChunkSelected = true;

                            }

                            selectedChunk->setBlock(chunkLinearIndex++, number);

                            break;
                    
                    }

                }
                else
                    word += saveData[i];

            }

            t.clean();
            logger::debugLog("Time: " + std::to_string(t.getDurationMs()) + " ms");


        }
        else
            logger::errorLog("Terrain file " + truePath + " was not found");

    }

    void chunkManager::highPriorityUpdate(const vec3& chunkPos) {

        chunk* selectedChunk = selectChunkByChunkPos(chunkPos);


        std::unique_lock<std::recursive_mutex> lock(priorityMeshingListMutex_);

        if (selectedChunk)
            priorityMeshingList_.push_back(chunkPos);

    }

    void chunkManager::selectAIworld(unsigned int individualID) {

        if (AImodeON_) {

            originalWorldAccess_ = false;
            selectedAIWorld_ = individualID;

        }
        else
            logger::errorLog("AI mode needs to be enabled to select an AI agent world");

    }

    void chunkManager::selectOriginalWorld() {

        if (AImodeON_)
            originalWorldAccess_ = true;
        else
            logger::errorLog("AI mode needs to be enabled to select an AI agent world");

    }

    void chunkManager::cleanUp() {

        for (auto it = chunks_.begin(); it != chunks_.end(); it++)
            if (it->second)
                delete it->second;
        chunks_.clear();

        if (drawableChunksRead_) {
        
            delete drawableChunksRead_;
            drawableChunksRead_ = nullptr;

        }
       
        if (drawableChunksWrite_) {

            delete drawableChunksWrite_;
            drawableChunksWrite_ = nullptr;

        }

        for (auto it = freeChunks_.cbegin(); it != freeChunks_.cend(); it++)
            if (*it)
                delete* it;
        freeChunks_.clear();

        freeableChunks_.clear();

        priorityMeshingList_.clear();
        priorityUpdateList_.clear();

        for (auto it = AIagentChunks_.cbegin(); it != AIagentChunks_.cend(); it++)
            for (auto itChunks = it->second.cbegin(); itChunks != it->second.cend(); itChunks++)
                delete itChunks->second;
        AIagentChunks_.clear();

        initialised_ = false;

    }

}
