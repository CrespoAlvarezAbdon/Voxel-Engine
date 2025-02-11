#include "chunk.h"

#include <algorithm>
#include <ctime>
#include <cstdlib>
#include <cstring>
#include <cmath>
#include <chrono>
#include <fstream>
#include <format>
#include <filesystem>
#include <iostream>
#include <stdexcept>

#include <AIAPI.h>
#include <camera.h>
#include <player.h>
#include <input.h>
#include <gui.h>
#include <game.h>
#include <Time/Timer/timer.h>
#include <Graphics/graphics.h>
#include <Graphics/Lighting/Lights/light.h>
#include <Graphics/Lighting/Lights/DirectionalLight/directionalLight.h>
#include <Graphics/Lighting/Lights/PointLight/pointLight.h>
#include <Graphics/Lighting/Lights/SpotLight/spotLight.h>
#include <Graphics/Vertex/ChunkVertexBuffer/chunkVertexBuffer.h>
#include <Registry/registries.h>
#include <Utilities/Logger/logger.h>
#include <Utilities/Var/var.h>

#include <AI/AIGameEx1.h>


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
    const modelNormals* chunk::blockNormals_ = nullptr;


    void chunk::init() {
    
        if (initialised_)
            logger::errorLog("Chunk class's static member are already initialised");
        else {
        
            blockVertices_ = &models::getModelAt(1);
            blockTriangles_ = &models::getModelTrianglesAt(1);
            blockNormals_ = &models::getModelNormalsAt(1);

            initialised_ = true;
        
        }

    }

    chunk::chunk()
    : modified_(false),
      needsRemesh_(false),
      nBlocks_(0),
      nBlocksPlusX_(0),
      nBlocksMinusX_(0),
      nBlocksPlusY_(0),
      nBlocksMinusY_(0),
      nBlocksPlusZ_(0),
      nBlocksMinusZ_(0),
      nNeighbors_(0),
      loadLevel_(chunkStatus::NOTLOADED),
      chunkPos_(vec3Zero) {

        std::memset(blocksLocalIDs_, 0, nBlocksChunk * sizeof(unsigned short));

        std::memset(blockLight_, 0, nBlocksChunk * sizeof(bool));

        std::memset(neighborBlocksPlusX_, 0, nBlocksChunkEdge * sizeof(unsigned short));
        std::memset(neighborBlocksMinusX_, 0, nBlocksChunkEdge * sizeof(unsigned short));
        std::memset(neighborBlocksPlusY_, 0, nBlocksChunkEdge * sizeof(unsigned short));
        std::memset(neighborBlocksMinusY_, 0, nBlocksChunkEdge * sizeof(unsigned short));
        std::memset(neighborBlocksPlusZ_, 0, nBlocksChunkEdge * sizeof(unsigned short));
        std::memset(neighborBlocksMinusZ_, 0, nBlocksChunkEdge * sizeof(unsigned short));
    
    }
   
    chunk::chunk(bool empty, const vec3& chunkPos)
    : modified_(false),
      needsRemesh_(false),
      nBlocks_(0),
      nBlocksPlusX_(0),
      nBlocksMinusX_(0),
      nBlocksPlusY_(0),
      nBlocksMinusY_(0),
      nBlocksPlusZ_(0),
      nBlocksMinusZ_(0),
      nNeighbors_(0),
      loadLevel_(chunkStatus::NOTLOADED),
      chunkPos_(vec3Zero) {

        std::memset(blocksLocalIDs_, 0, nBlocksChunk * sizeof(unsigned short));

        std::memset(blockLight_, 0, nBlocksChunk * sizeof(bool));

        std::memset(neighborBlocksPlusX_, 0, nBlocksChunkEdge * sizeof(unsigned short));
        std::memset(neighborBlocksMinusX_, 0, nBlocksChunkEdge * sizeof(unsigned short));
        std::memset(neighborBlocksPlusY_, 0, nBlocksChunkEdge * sizeof(unsigned short));
        std::memset(neighborBlocksMinusY_, 0, nBlocksChunkEdge * sizeof(unsigned short));
        std::memset(neighborBlocksPlusZ_, 0, nBlocksChunkEdge * sizeof(unsigned short));
        std::memset(neighborBlocksMinusZ_, 0, nBlocksChunkEdge * sizeof(unsigned short));
        
        if (!empty)
            worldGen::generate(*this); // This can only call to setBlock to modify the chunk and that method already takes care of 'blocksMutex_'.
        
    }

    chunk::chunk(const chunk& c)
    : modified_(c.modified_),
      needsRemesh_(c.needsRemesh_.load()),
      nBlocks_(c.nBlocks_.load()),
      nBlocksPlusX_(c.nBlocksPlusX_.load()),
      nBlocksMinusX_(c.nBlocksMinusX_.load()),
      nBlocksPlusY_(c.nBlocksPlusY_.load()),
      nBlocksMinusY_(c.nBlocksMinusY_.load()),
      nBlocksPlusZ_(c.nBlocksPlusZ_.load()),
      nBlocksMinusZ_(c.nBlocksMinusZ_.load()),
      nNeighbors_(c.nNeighbors_),
      loadLevel_(c.loadLevel_.load()),
      chunkPos_(c.chunkPos_) {

        std::unique_lock<std::shared_mutex> lock(blocksMutex_);

        renderingData_.globalChunkPos = c.renderingData_.globalChunkPos;

        palette_ = c.palette_;
        paletteCount_ = c.paletteCount_;
        freeLocalIDs_ = c.freeLocalIDs_;

        std::memcpy(blocksLocalIDs_, c.blocksLocalIDs_, nBlocksChunk * sizeof(unsigned short));

        std::memcpy(blockLight_, c.blockLight_, nBlocksChunk * sizeof(bool));

        floodPointLightPositions_ = c.floodPointLightPositions_;

        std::memcpy(neighborBlocksPlusX_, c.neighborBlocksPlusX_, nBlocksChunkEdge * sizeof(unsigned short));
        std::memcpy(neighborBlocksMinusX_, c.neighborBlocksMinusX_, nBlocksChunkEdge * sizeof(unsigned short));
        std::memcpy(neighborBlocksPlusY_, c.neighborBlocksPlusY_, nBlocksChunkEdge * sizeof(unsigned short));
        std::memcpy(neighborBlocksMinusY_, c.neighborBlocksMinusY_, nBlocksChunkEdge * sizeof(unsigned short));
        std::memcpy(neighborBlocksPlusZ_, c.neighborBlocksPlusZ_, nBlocksChunkEdge * sizeof(unsigned short));
        std::memcpy(neighborBlocksMinusZ_, c.neighborBlocksMinusZ_, nBlocksChunkEdge * sizeof(unsigned short));

        renderingData_.vertices = c.renderingData_.vertices;

    }

    const block& chunk::getBlock(GLbyte x, GLbyte y, GLbyte z) {

        std::shared_lock<std::shared_mutex> lock(blocksMutex_);

        unsigned int localID = blocksLocalIDs_[x][y][z];

        return localID ? block::getBlockC(palette_.getT2(localID)) : block::emptyBlock();

    }

    const block& chunk::setBlock(GLbyte x, GLbyte y, GLbyte z, const block& b, bool modification) {

        unsigned short& actualLocalID = blocksLocalIDs_[x][y][z];
        unsigned short oldLocalID = actualLocalID;
        unsigned int oldGlobalID = actualLocalID ? palette_.getT2(actualLocalID) : 0;
 
        placeNewBlock(actualLocalID, b);

        bool blockWasModified = oldLocalID != actualLocalID;

        needsRemesh_ = needsRemesh_ || blockWasModified;

        modified_ = modified_ || (blockWasModified && modification);

        if (!oldLocalID && actualLocalID)
            nBlocks_++;
        else if (oldLocalID && !actualLocalID)
            nBlocks_--;

        // Update block light information.
        const block& oldB = block::getBlockC(oldGlobalID);
        const varRef& oldEmittedLight = oldB.emittedLight();
        const varRef& emittedLight = b.emittedLight();

        if (!oldEmittedLight.isNull()) { // Remove old light pos.

            if (oldEmittedLight.getVarType() == var::varType::POINTLIGHT) {

                floodPointLightPositions_.erase(vec3{ x,y,z });

            }
            else if (oldEmittedLight.getVarType() == var::varType::SPOTLIGHT) {

                throw std::runtime_error("This is not implemented yet");

            }

        }

        if (!emittedLight.isNull()) { // Add new light pos.

            if (emittedLight.getVarType() == var::varType::POINTLIGHT) {

                floodPointLightPositions_.insert(vec3{ x,y,z });

            }
            else if (emittedLight.getVarType() == var::varType::SPOTLIGHT) {

                throw std::runtime_error("This is not implemented yet");

            }
            else if (emittedLight.getVarType() == var::varType::DIRECTIONALLIGHT)
                throw std::runtime_error("Directional lights cannot be applied by blocks");
            else
                throw std::runtime_error("Unknown light type for block specified (varType number is " + std::to_string((int)emittedLight.getVarType()) + ")");

        }

        return oldB;

    }

    // TODO. MOVER AL FINAL DEL FICHERO.
    void chunk::placeNewBlock(unsigned short& actualLocalID, const block& newBlock) {

        if (newBlock.name() == "starminer::marbleBlock2") 
        {
            int a = 3 + 2;
        }

        unsigned int newGlobalID = newBlock.intID(),
                     oldGlobalID = actualLocalID ? palette_.getT2(actualLocalID) : 0;
        needsRemesh_ = needsRemesh_ || oldGlobalID != newGlobalID;

        int a = -1;
        if (actualLocalID)
            a = palette_.getT2(actualLocalID);

        if (actualLocalID) {

            if (paletteCount_.at(actualLocalID) == 1) { // The old local ID is no longer used at (x,y,z).

                palette_.eraseT1(actualLocalID);
                paletteCount_.erase(actualLocalID);
                freeLocalIDs_.insert(actualLocalID);

            }
            else
                paletteCount_.at(actualLocalID)--;

        }

        if (newGlobalID == 0) // The new block is an empty block.
            actualLocalID = 0;
        else {

            if (palette_.containsT2(newGlobalID)) { // The new block already has a relation in the palette.

                actualLocalID = palette_.getT1(newGlobalID); 
                paletteCount_.at(actualLocalID)++;

            }
            else { // The new block does not have an associated local ID in the palette.

                if (freeLocalIDs_.empty()) { 

                    actualLocalID = palette_.size() + 1;

                }
                else {

                    actualLocalID = *freeLocalIDs_.begin();
                    freeLocalIDs_.erase(actualLocalID);

                }

                palette_.insert(actualLocalID, newGlobalID);
                paletteCount_[actualLocalID] = 1;

            }

        }

    }

    void chunk::setBlockNeighbor(unsigned int firstIndex, unsigned int secondIndex, blockViewDir neighbor, const block& block, bool modification, unsigned int LOD) {

        unsigned short oldLocalID = 0;
        bool blockWasModified = false;
        unsigned short(*neighborBlocks)[CHUNK_SIZE][CHUNK_SIZE] = nullptr;
        
        if (neighbor == blockViewDir::NONE)
            logger::errorLog("No block view direction was specified");
        else if (neighbor == blockViewDir::PLUSX) {

            unsigned short& actualLocalID = neighborBlocksPlusX_[firstIndex][secondIndex];
            oldLocalID = actualLocalID;
            placeNewBlock(actualLocalID, block);
            blockWasModified = oldLocalID != actualLocalID;
            needsRemesh_ = needsRemesh_ || blockWasModified && blocksLocalIDs_[CHUNK_SIZE_LIMIT][firstIndex][secondIndex];
            if (!oldLocalID && actualLocalID)
                nBlocksPlusX_++;
            else if (oldLocalID && !actualLocalID)
                nBlocksPlusX_--;

        }
        else if (neighbor == blockViewDir::NEGX) {

            if (LOD == 1) {

                neighborBlocks = &neighborBlocksMinusX_;

            }
            else if (LOD == 2) {

                neighborBlocks = &neighborBlocksMinusX_;

            }
            else
                logger::errorLog("Unsupported LOD " + std::to_string(LOD));

            unsigned short& actualLocalID = (*neighborBlocks)[firstIndex][secondIndex];
            oldLocalID = actualLocalID;
            placeNewBlock(actualLocalID, block);
            blockWasModified = oldLocalID != actualLocalID;
            needsRemesh_ = needsRemesh_ || blockWasModified && blocksLocalIDs_[0][firstIndex][secondIndex];
            if (!oldLocalID && actualLocalID)
                nBlocksMinusX_++;
            else if (oldLocalID && !actualLocalID)
                nBlocksMinusX_--;
            
        }
        else if (neighbor == blockViewDir::PLUSY) {

            unsigned short& actualLocalID = neighborBlocksPlusY_[firstIndex][secondIndex];
            oldLocalID = actualLocalID;
            placeNewBlock(actualLocalID, block);
            blockWasModified = oldLocalID != actualLocalID;
            needsRemesh_ = needsRemesh_ || blockWasModified && blocksLocalIDs_[firstIndex][CHUNK_SIZE_LIMIT][secondIndex];
            if (!oldLocalID && actualLocalID)
                nBlocksPlusY_++;
            else if (oldLocalID && !actualLocalID)
                nBlocksPlusY_--;

        }
        else if (neighbor == blockViewDir::NEGY) {

            if (LOD == 1) {

                neighborBlocks = &neighborBlocksMinusY_;

            }
            else if (LOD == 2) {

                neighborBlocks = &neighborBlocksMinusY_;

            }
            else
                logger::errorLog("Unsupported LOD " + std::to_string(LOD));

            unsigned short& actualLocalID = (*neighborBlocks)[firstIndex][secondIndex];
            oldLocalID = actualLocalID;
            placeNewBlock(actualLocalID, block);
            blockWasModified = oldLocalID != actualLocalID;
            needsRemesh_ = needsRemesh_ || blockWasModified && blocksLocalIDs_[firstIndex][0][secondIndex];
            if (!oldLocalID && actualLocalID)
                nBlocksMinusY_++;
            else if (oldLocalID && !actualLocalID)
                nBlocksMinusY_--;

        }
        else if (neighbor == blockViewDir::PLUSZ) {

            unsigned short& actualLocalID = neighborBlocksPlusZ_[firstIndex][secondIndex];
            oldLocalID = actualLocalID;
            placeNewBlock(actualLocalID, block);
            blockWasModified = oldLocalID != actualLocalID;
            needsRemesh_ = needsRemesh_ || blockWasModified && blocksLocalIDs_[firstIndex][secondIndex][CHUNK_SIZE_LIMIT];
            if (!oldLocalID && actualLocalID)
                nBlocksPlusZ_++;
            else if (oldLocalID && !actualLocalID)
                nBlocksPlusZ_--;

        }
        else if (neighbor == blockViewDir::NEGZ) {

            if (LOD == 1) {

                neighborBlocks = &neighborBlocksMinusZ_;

            }
            else if (LOD == 2) {

                neighborBlocks = &neighborBlocksMinusZ_;

            }
            else
                logger::errorLog("Unsupported LOD " + std::to_string(LOD));

            unsigned short& actualLocalID = (*neighborBlocks)[firstIndex][secondIndex];
            oldLocalID = actualLocalID;
            placeNewBlock(actualLocalID, block);
            blockWasModified = oldLocalID != actualLocalID;
            needsRemesh_ = needsRemesh_ || blockWasModified && blocksLocalIDs_[firstIndex][secondIndex][0];
            if (!oldLocalID && actualLocalID)
                nBlocksMinusZ_++;
            else if (oldLocalID && !actualLocalID)
                nBlocksMinusZ_--;

        }
        else
            logger::errorLog("Unsupported block view direction specified");

        modified_ = modified_ || (blockWasModified && modification);

    }

    void chunk::chunkPos(const vec3& newChunkPos) {

        chunkPos_ = newChunkPos;
        renderingData_.globalChunkPos.x = newChunkPos.x * CHUNK_SIZE + CHUNK_SIZE / 2;
        renderingData_.globalChunkPos.y = newChunkPos.y * CHUNK_SIZE + CHUNK_SIZE / 2;
        renderingData_.globalChunkPos.z = newChunkPos.z * CHUNK_SIZE + CHUNK_SIZE / 2;

    }

    bool chunk::renewMesh() {

        std::unique_lock<std::shared_mutex> lock(renderingDataMutex_);

        if (needsRemesh_) {

            needsRemesh_ = false;

            model* chunkModel = nullptr;

            renderingData_.vertices = model();
            renderingData_.translucentVertices = model();
            renderingData_.pointLights = std::vector<lightInstance>();
            renderingData_.spotLights = std::vector<lightInstance>();

            std::memset(blockLight_, 0, 4096 * sizeof(bool));

            bool blockLightChecked[16][16][16];

            // Read chunk data section starts.
            blocksMutex_.lock_shared();

            // COSAS QUE HACER.
            // 1º. SMOOTH LIGHTING QUE HACE MINECRAFT.
            // 2º. SOPORTE PARA LOS BORDES DE LOS CHUNKS.
            // 3º. SOPORTE PARA QUE UNA LUZ AFECTE A VARIOS CHUNKS.

            int x = 0,
                y = 0,
                z = 0;
            unsigned short localID = 0;

            // Update block light render information.
            std::deque<std::pair<vec3, unsigned char>> floodLightPositions;
            for (auto it = floodPointLightPositions_.cbegin(); it != floodPointLightPositions_.cend(); it++) {

                x = it->x;
                y = it->y;
                z = it->z;

                localID = blocksLocalIDs_[x][y][z];
                block& b = localID ? block::getBlockC(palette_.getT2(localID)) : block::emptyBlock();
                const varRef& emittedLight = b.emittedLight();

                // Add block's light.
                if (!emittedLight.isNull()) {

                    if (emittedLight.getVarType() == var::varType::POINTLIGHT) {

                        lightInstance& instance = renderingData_.pointLights.emplace_back();
                        instance.pos = getGlobalPos(chunkPos_.x, chunkPos_.y, chunkPos_.z, x, y, z);
                        instance.lightTypeIndex = b.emittedLightIndex();

                        // Search for blocks affected by this light.
                        std::memset(blockLightChecked, 0, nBlocksChunk * sizeof(bool));
                        floodLightPositions.clear();
                        floodLightPositions.emplace_back(vec3{ x, y, z }, 7);
                        while (floodLightPositions.size() > 0) {

                            //std::cout << std::to_string(floodLightPositions.size()) << std::endl;
                            const vec3& pos = floodLightPositions.front().first;
                            //std::cout << std::to_string(pos) << std::endl;
                            unsigned char lightLevelToApply = floodLightPositions.front().second;
                            //std::cout << "light " << std::to_string(lightLevelToApply) << std::endl;

                            if (lightLevelToApply > 0 && !blockLightChecked[(int)pos.x][(int)pos.y][(int)pos.z]) {

                                blockLight_[(int)pos.x][(int)pos.y][(int)pos.z] = lightLevelToApply;

                                //+x
                                if (pos.x < CHUNK_SIZE_LIMIT && blocksLocalIDs_[(int)pos.x + 1][(int)pos.y][(int)pos.z] == 0 &&
                                    blockLight_[(int)pos.x + 1][(int)pos.y][(int)pos.z] + 1 < lightLevelToApply) {

                                    floodLightPositions.emplace_back(vec3{ pos.x + 1, pos.y, pos.z }, lightLevelToApply - 1);

                                }

                                //-x
                                if (pos.x > 0 && blocksLocalIDs_[(int)pos.x - 1][(int)pos.y][(int)pos.z] == 0 &&
                                    blockLight_[(int)pos.x - 1][(int)pos.y][(int)pos.z] + 1 < lightLevelToApply) {

                                    floodLightPositions.emplace_back(vec3{ pos.x - 1, pos.y, pos.z }, lightLevelToApply - 1);

                                }

                                //+y
                                if (pos.y < CHUNK_SIZE_LIMIT && blocksLocalIDs_[(int)pos.x][(int)pos.y + 1][(int)pos.z] == 0 &&
                                    blockLight_[(int)pos.x][(int)pos.y + 1][(int)pos.z] + 1 < lightLevelToApply) {

                                    floodLightPositions.emplace_back(vec3{ pos.x, pos.y + 1, pos.z }, lightLevelToApply - 1);

                                }

                                //-y
                                if (pos.y > 0 && blocksLocalIDs_[(int)pos.x][(int)pos.y - 1][(int)pos.z] == 0 &&
                                    blockLight_[(int)pos.x][(int)pos.y - 1][(int)pos.z] + 1 < lightLevelToApply) {

                                    floodLightPositions.emplace_back(vec3{ pos.x, pos.y - 1, pos.z }, lightLevelToApply - 1);

                                }

                                //+z
                                if (pos.z < CHUNK_SIZE_LIMIT && blocksLocalIDs_[(int)pos.x][(int)pos.y][(int)pos.z + 1] == 0 &&
                                    blockLight_[(int)pos.x][(int)pos.y][(int)pos.z + 1] + 1 < lightLevelToApply) {

                                    floodLightPositions.emplace_back(vec3{ pos.x, pos.y, pos.z + 1 }, lightLevelToApply - 1);

                                }

                                //-z
                                if (pos.z > 0 && blocksLocalIDs_[(int)pos.x][(int)pos.y][(int)pos.z - 1] == 0 &&
                                    blockLight_[(int)pos.x][(int)pos.y][(int)pos.z - 1] + 1 < lightLevelToApply) {

                                    floodLightPositions.emplace_back(vec3{ pos.x, pos.y, pos.z - 1 }, lightLevelToApply - 1);

                                }

                            }

                            blockLightChecked[(int)pos.x][(int)pos.y][(int)pos.z] = true;

                            floodLightPositions.pop_front();

                        }

                    }
                    else if (emittedLight.getVarType() == var::varType::SPOTLIGHT) {

                        lightInstance& instance = renderingData_.spotLights.emplace_back();
                        instance.pos = getGlobalPos(chunkPos_.x, chunkPos_.y, chunkPos_.z, x, y, z);
                        instance.lightTypeIndex = b.emittedLightIndex();

                    }
                    else if (emittedLight.getVarType() == var::varType::DIRECTIONALLIGHT)
                        throw std::runtime_error("Directional lights cannot be applied by blocks");
                    else
                        throw std::runtime_error("Unknown light type for block specified (varType number is " + std::to_string((int)emittedLight.getVarType()) + ")");

                }

            }

            // Render faces that do not require data from neighbor chunks.
            bool blockHasLight = false;
            vertex aux;
            const block* bNeighbor = nullptr;
            unsigned short neighborLocalID = 0;
            if (nBlocks_ && nBlocks_ < nBlocksChunk)
                for (x = 0; x < CHUNK_SIZE; x++)
                    for (y = 0; y < CHUNK_SIZE; y++)
                        for (z = 0; z < CHUNK_SIZE; z++) {

                            localID = blocksLocalIDs_[x][y][z];
                            block& b = localID ? block::getBlockC(palette_.getT2(localID)) : block::emptyBlock();
                            const varRef& emittedLight = b.emittedLight();

                            blockHasLight = blockLight_[x][y][z] > 0;

                            // Add block's model to the mesh if necessary.
                            if (b.opacity() <= blockOpacity::TRANSLUCENTBLOCK) {

                                // z+
                                if (z < CHUNK_SIZE_LIMIT && (neighborLocalID = blocksLocalIDs_[x][y][z + 1])) {

                                    bNeighbor = &block::getBlockC(palette_.getT2(neighborLocalID));

                                    if (b != *bNeighbor) {
                                    
                                        chunkModel = (bNeighbor->opacity() == blockOpacity::TRANSLUCENTBLOCK) ? &renderingData_.translucentVertices : &renderingData_.vertices;

                                        // Create the face's vertices for face z-.
                                        for (int vertex = 0; vertex < blockTriangles_->operator[](0).size(); vertex++) {

                                            aux.positions[0] = chunkPos_.x * CHUNK_SIZE + x + blockVertices_->operator[](blockTriangles_->operator[](0)[vertex]).positions[0];
                                            aux.positions[1] = chunkPos_.y * CHUNK_SIZE + y + blockVertices_->operator[](blockTriangles_->operator[](0)[vertex]).positions[1];
                                            aux.positions[2] = chunkPos_.z * CHUNK_SIZE + z + 1 + blockVertices_->operator[](blockTriangles_->operator[](0)[vertex]).positions[2];
                                            aux.additionalData[0] = bNeighbor->getMaterialIndex();
                                            aux.additionalData[1] = blockHasLight ? 255 : 0;

                                            chunkModel->push_back(aux);

                                        }

                                        // Add texture to the face.
                                        models::addBlockFaceTexture(*bNeighbor, *chunkModel, "faceZ-");
                                    
                                    }

                                }

                                // z-
                                if (z > 0 && (neighborLocalID = blocksLocalIDs_[x][y][z - 1])) {

                                    bNeighbor = &block::getBlockC(palette_.getT2(neighborLocalID));

                                    if (b != *bNeighbor) {
                                    
                                        chunkModel = (bNeighbor->opacity() == blockOpacity::TRANSLUCENTBLOCK) ? &renderingData_.translucentVertices : &renderingData_.vertices;

                                        // Create the face's vertices for z+.
                                        for (int vertex = 0; vertex < blockTriangles_->operator[](0).size(); vertex++) {

                                            aux.positions[0] = chunkPos_.x * CHUNK_SIZE + x + blockVertices_->operator[](blockTriangles_->operator[](1)[vertex]).positions[0];
                                            aux.positions[1] = chunkPos_.y * CHUNK_SIZE + y + blockVertices_->operator[](blockTriangles_->operator[](1)[vertex]).positions[1];
                                            aux.positions[2] = chunkPos_.z * CHUNK_SIZE + z - 1 + blockVertices_->operator[](blockTriangles_->operator[](1)[vertex]).positions[2];
                                            aux.additionalData[0] = bNeighbor->getMaterialIndex();
                                            aux.additionalData[1] = blockHasLight ? 255 : 0;

                                            chunkModel->push_back(aux);

                                        }

                                        // Add texture to the face.
                                        models::addBlockFaceTexture(*bNeighbor, *chunkModel, "faceZ+");
                                    
                                    }

                                }
                               
                                // y+
                                if (y < CHUNK_SIZE_LIMIT && (neighborLocalID = blocksLocalIDs_[x][y + 1][z])) {

                                    bNeighbor = &block::getBlockC(palette_.getT2(neighborLocalID));

                                    if (b != *bNeighbor) {

                                        chunkModel = (bNeighbor->opacity() == blockOpacity::TRANSLUCENTBLOCK) ? &renderingData_.translucentVertices : &renderingData_.vertices;

                                        // Create the face's vertices for face y-.
                                        for (int vertex = 0; vertex < blockTriangles_->operator[](0).size(); vertex++) {

                                            aux.positions[0] = chunkPos_.x * CHUNK_SIZE + x + blockVertices_->operator[](blockTriangles_->operator[](3)[vertex]).positions[0];
                                            aux.positions[1] = chunkPos_.y * CHUNK_SIZE + y + 1 + blockVertices_->operator[](blockTriangles_->operator[](3)[vertex]).positions[1];
                                            aux.positions[2] = chunkPos_.z * CHUNK_SIZE + z + blockVertices_->operator[](blockTriangles_->operator[](3)[vertex]).positions[2];
                                            aux.additionalData[0] = bNeighbor->getMaterialIndex();
                                            aux.additionalData[1] = blockHasLight ? 255 : 0;

                                            chunkModel->push_back(aux);

                                        }

                                        // Add texture to the face.
                                        models::addBlockFaceTexture(*bNeighbor, *chunkModel, "faceY-");

                                    }

                                }
                                
                                // y-
                                if (y > 0 && (neighborLocalID = blocksLocalIDs_[x][y - 1][z])) {

                                    bNeighbor = &block::getBlockC(palette_.getT2(neighborLocalID));

                                    if (b != *bNeighbor) {

                                        chunkModel = (bNeighbor->opacity() == blockOpacity::TRANSLUCENTBLOCK) ? &renderingData_.translucentVertices : &renderingData_.vertices;

                                        // Create the face's vertices for face y+.
                                        for (int vertex = 0; vertex < blockTriangles_->operator[](0).size(); vertex++) {

                                            aux.positions[0] = chunkPos_.x * CHUNK_SIZE + x + blockVertices_->operator[](blockTriangles_->operator[](2)[vertex]).positions[0];
                                            aux.positions[1] = chunkPos_.y * CHUNK_SIZE + y - 1 + blockVertices_->operator[](blockTriangles_->operator[](2)[vertex]).positions[1];
                                            aux.positions[2] = chunkPos_.z * CHUNK_SIZE + z + blockVertices_->operator[](blockTriangles_->operator[](2)[vertex]).positions[2];
                                            aux.additionalData[0] = bNeighbor->getMaterialIndex();
                                            aux.additionalData[1] = blockHasLight ? 255 : 0;

                                            chunkModel->push_back(aux);

                                        }

                                        // Add texture to the face.
                                        models::addBlockFaceTexture(*bNeighbor, *chunkModel, "faceY+");

                                    }

                                }

                                // x+
                                if (x < CHUNK_SIZE_LIMIT && (neighborLocalID = blocksLocalIDs_[x + 1][y][z])) {

                                    bNeighbor = &block::getBlockC(palette_.getT2(neighborLocalID));

                                    if (b != *bNeighbor) {

                                        chunkModel = (bNeighbor->opacity() == blockOpacity::TRANSLUCENTBLOCK) ? &renderingData_.translucentVertices : &renderingData_.vertices;

                                        // Create the face's vertices for face x-.
                                        for (int vertex = 0; vertex < blockTriangles_->operator[](0).size(); vertex++) {

                                            aux.positions[0] = chunkPos_.x * CHUNK_SIZE + x + 1 + blockVertices_->operator[](blockTriangles_->operator[](4)[vertex]).positions[0];
                                            aux.positions[1] = chunkPos_.y * CHUNK_SIZE + y + blockVertices_->operator[](blockTriangles_->operator[](4)[vertex]).positions[1];
                                            aux.positions[2] = chunkPos_.z * CHUNK_SIZE + z + blockVertices_->operator[](blockTriangles_->operator[](4)[vertex]).positions[2];
                                            aux.additionalData[0] = bNeighbor->getMaterialIndex();
                                            aux.additionalData[1] = blockHasLight ? 255 : 0;

                                            chunkModel->push_back(aux);

                                        }

                                        // Add texture to the face.
                                        models::addBlockFaceTexture(*bNeighbor, *chunkModel, "faceX-");

                                    }

                                }

                                // x-
                                if (x > 0 && (neighborLocalID = blocksLocalIDs_[x - 1][y][z])) {

                                    bNeighbor = &block::getBlockC(palette_.getT2(neighborLocalID));

                                    if (b != *bNeighbor) {

                                        chunkModel = (bNeighbor->opacity() == blockOpacity::TRANSLUCENTBLOCK) ? &renderingData_.translucentVertices : &renderingData_.vertices;

                                        // Create the face's vertices for face x+.
                                        for (int vertex = 0; vertex < blockTriangles_->operator[](0).size(); vertex++) {

                                            aux.positions[0] = chunkPos_.x * CHUNK_SIZE + x - 1 + blockVertices_->operator[](blockTriangles_->operator[](5)[vertex]).positions[0];
                                            aux.positions[1] = chunkPos_.y * CHUNK_SIZE + y + blockVertices_->operator[](blockTriangles_->operator[](5)[vertex]).positions[1];
                                            aux.positions[2] = chunkPos_.z * CHUNK_SIZE + z + blockVertices_->operator[](blockTriangles_->operator[](5)[vertex]).positions[2];
                                            aux.additionalData[0] = bNeighbor->getMaterialIndex();
                                            aux.additionalData[1] = blockHasLight ? 255 : 0;

                                            chunkModel->push_back(aux);

                                        }

                                        // Add texture to the face.
                                        models::addBlockFaceTexture(*bNeighbor, *chunkModel, "faceX+");

                                    }

                                }

                            }

                        }

            if (nBlocksPlusZ_) {
            
                for (x = 0; x < CHUNK_SIZE; x++)
                    for (y = 0; y < CHUNK_SIZE; y++) {

                        // LOD 1.
                        localID = blocksLocalIDs_[x][y][CHUNK_SIZE_LIMIT];
                        const block& b = localID ? block::getBlockC(palette_.getT2(localID)) : block::emptyBlock();

                        if (b.opacity() <= blockOpacity::TRANSLUCENTBLOCK && (neighborLocalID = neighborBlocksPlusZ_[x][y])) {

                            // Front face vertices with culling of non-visible faces. z-
                            bNeighbor = &block::getBlockC(palette_.getT2(neighborLocalID));

                            if (b != *bNeighbor) {

                                chunkModel = (bNeighbor->opacity() == blockOpacity::TRANSLUCENTBLOCK) ? &renderingData_.translucentVertices : &renderingData_.vertices;

                                // Create the face's vertices for face z-.
                                for (int vertex = 0; vertex < blockTriangles_->operator[](0).size(); vertex++) {

                                    aux.positions[0] = chunkPos_.x * CHUNK_SIZE + x + blockVertices_->operator[](blockTriangles_->operator[](0)[vertex]).positions[0];
                                    aux.positions[1] = chunkPos_.y * CHUNK_SIZE + y + blockVertices_->operator[](blockTriangles_->operator[](0)[vertex]).positions[1];
                                    aux.positions[2] = (chunkPos_.z + 1) * CHUNK_SIZE + blockVertices_->operator[](blockTriangles_->operator[](0)[vertex]).positions[2];
                                    aux.additionalData[0] = bNeighbor->getMaterialIndex();

                                    chunkModel->push_back(aux);

                                }

                                // Add texture to the face.
                                models::addBlockFaceTexture(*bNeighbor, *chunkModel, "faceZ-");

                            }

                        }

                    }
            
            }

            if (nBlocksMinusZ_) {

                for (x = 0; x < CHUNK_SIZE; x++)
                    for (y = 0; y < CHUNK_SIZE; y++) {

                        localID = blocksLocalIDs_[x][y][0];
                        const block& b = localID ? block::getBlockC(palette_.getT2(localID)) : block::emptyBlock();

                        if (b.opacity() <= blockOpacity::TRANSLUCENTBLOCK && (neighborLocalID = neighborBlocksMinusZ_[x][y])) {

                            bNeighbor = &block::getBlockC(palette_.getT2(neighborLocalID));

                            if (b != *bNeighbor) {

                                chunkModel = (bNeighbor->opacity() == blockOpacity::TRANSLUCENTBLOCK) ? &renderingData_.translucentVertices : &renderingData_.vertices;

                                // Create the face's vertices for z+.
                                for (int vertex = 0; vertex < blockTriangles_->operator[](0).size(); vertex++) {

                                    aux.positions[0] = chunkPos_.x * CHUNK_SIZE + x + blockVertices_->operator[](blockTriangles_->operator[](1)[vertex]).positions[0];
                                    aux.positions[1] = chunkPos_.y * CHUNK_SIZE + y + blockVertices_->operator[](blockTriangles_->operator[](1)[vertex]).positions[1];
                                    aux.positions[2] = (chunkPos_.z - 1) * CHUNK_SIZE + (16 - 1) + blockVertices_->operator[](blockTriangles_->operator[](1)[vertex]).positions[2];
                                    aux.additionalData[0] = bNeighbor->getMaterialIndex();

                                    chunkModel->push_back(aux);

                                }

                                // Add texture to the face.
                                models::addBlockFaceTexture(*bNeighbor, *chunkModel, "faceZ+");

                            }

                        }

                    }

            }

            if (nBlocksPlusY_) {

                for (x = 0; x < CHUNK_SIZE; x++)
                    for (z = 0; z < CHUNK_SIZE; z++) {

                        localID = blocksLocalIDs_[x][CHUNK_SIZE_LIMIT][z];
                        const block& b = localID ? block::getBlockC(palette_.getT2(localID)) : block::emptyBlock();

                        if (b.opacity() <= blockOpacity::TRANSLUCENTBLOCK && (neighborLocalID = neighborBlocksPlusY_[x][z])) {

                            bNeighbor = &block::getBlockC(palette_.getT2(neighborLocalID));

                            if (b != *bNeighbor) {

                                chunkModel = (bNeighbor->opacity() == blockOpacity::TRANSLUCENTBLOCK) ? &renderingData_.translucentVertices : &renderingData_.vertices;

                                // Create the face's vertices for face y-.
                                for (int vertex = 0; vertex < blockTriangles_->operator[](0).size(); vertex++) {

                                    aux.positions[0] = chunkPos_.x * CHUNK_SIZE + x + blockVertices_->operator[](blockTriangles_->operator[](3)[vertex]).positions[0];
                                    aux.positions[1] = (chunkPos_.y + 1) * CHUNK_SIZE + blockVertices_->operator[](blockTriangles_->operator[](3)[vertex]).positions[1];
                                    aux.positions[2] = chunkPos_.z * CHUNK_SIZE + z + blockVertices_->operator[](blockTriangles_->operator[](3)[vertex]).positions[2];
                                    aux.additionalData[0] = bNeighbor->getMaterialIndex();

                                    chunkModel->push_back(aux);

                                }

                                // Add texture to the face.
                                models::addBlockFaceTexture(*bNeighbor, *chunkModel, "faceY-");

                            }

                        }

                    }

            }

            if (nBlocksMinusY_) {

                for (x = 0; x < CHUNK_SIZE; x++)
                    for (z = 0; z < CHUNK_SIZE; z++) {

                        localID = blocksLocalIDs_[x][0][z];
                        const block& b = localID ? block::getBlockC(palette_.getT2(localID)) : block::emptyBlock();

                        if (b.opacity() <= blockOpacity::TRANSLUCENTBLOCK && (neighborLocalID = neighborBlocksMinusY_[x][z])) {

                            bNeighbor = &block::getBlockC(palette_.getT2(neighborLocalID));

                            if (b != *bNeighbor) {

                                chunkModel = (bNeighbor->opacity() == blockOpacity::TRANSLUCENTBLOCK) ? &renderingData_.translucentVertices : &renderingData_.vertices;

                                // Create the face's vertices for face y+.
                                for (int vertex = 0; vertex < blockTriangles_->operator[](0).size(); vertex++) {

                                    aux.positions[0] = chunkPos_.x * CHUNK_SIZE + x + blockVertices_->operator[](blockTriangles_->operator[](2)[vertex]).positions[0];
                                    aux.positions[1] = (chunkPos_.y - 1) * CHUNK_SIZE + (16 - 1) + blockVertices_->operator[](blockTriangles_->operator[](2)[vertex]).positions[1];
                                    aux.positions[2] = chunkPos_.z * CHUNK_SIZE + z + blockVertices_->operator[](blockTriangles_->operator[](2)[vertex]).positions[2];
                                    aux.additionalData[0] = bNeighbor->getMaterialIndex();

                                    chunkModel->push_back(aux);

                                }

                                // Add texture to the face.
                                models::addBlockFaceTexture(*bNeighbor, *chunkModel, "faceY+");

                            }

                        }

                    }

            }

            if (nBlocksPlusX_) {

                for (y = 0; y < CHUNK_SIZE; y++)
                    for (z = 0; z < CHUNK_SIZE; z++) {

                        localID = blocksLocalIDs_[CHUNK_SIZE_LIMIT][y][z];
                        const block& b = localID ? block::getBlockC(palette_.getT2(localID)) : block::emptyBlock();

                        if (b.opacity() <= blockOpacity::TRANSLUCENTBLOCK && (neighborLocalID = neighborBlocksPlusX_[y][z])) {

                            bNeighbor = &block::getBlockC(palette_.getT2(neighborLocalID));

                            if (b != *bNeighbor) {

                                chunkModel = (bNeighbor->opacity() == blockOpacity::TRANSLUCENTBLOCK) ? &renderingData_.translucentVertices : &renderingData_.vertices;

                                // Create the face's vertices for face x-.
                                for (int vertex = 0; vertex < blockTriangles_->operator[](0).size(); vertex++) {

                                    aux.positions[0] = (chunkPos_.x + 1) * CHUNK_SIZE + blockVertices_->operator[](blockTriangles_->operator[](4)[vertex]).positions[0];
                                    aux.positions[1] = chunkPos_.y * CHUNK_SIZE + y + blockVertices_->operator[](blockTriangles_->operator[](4)[vertex]).positions[1];
                                    aux.positions[2] = chunkPos_.z * CHUNK_SIZE + z + blockVertices_->operator[](blockTriangles_->operator[](4)[vertex]).positions[2];
                                    aux.additionalData[0] = bNeighbor->getMaterialIndex();

                                    chunkModel->push_back(aux);

                                }

                                // Add texture to the face.
                                models::addBlockFaceTexture(*bNeighbor, *chunkModel, "faceX-");

                            }

                        }

                    }

            }

            if (nBlocksMinusX_) {

                for (y = 0; y < CHUNK_SIZE; y++)
                    for (z = 0; z < CHUNK_SIZE; z++) {

                        localID = blocksLocalIDs_[0][y][z];
                        const block& b = localID ? block::getBlockC(palette_.getT2(localID)) : block::emptyBlock();

                        if (b.opacity() <= blockOpacity::TRANSLUCENTBLOCK && (neighborLocalID = neighborBlocksMinusX_[y][z]))  {

                            bNeighbor = &block::getBlockC(palette_.getT2(neighborLocalID));

                            if (b != *bNeighbor) {

                                chunkModel = (bNeighbor->opacity() == blockOpacity::TRANSLUCENTBLOCK) ? &renderingData_.translucentVertices : &renderingData_.vertices;

                                // Create the face's vertices for face x+.
                                for (int vertex = 0; vertex < blockTriangles_->operator[](0).size(); vertex++) {

                                    aux.positions[0] = (chunkPos_.x - 1) * CHUNK_SIZE + (16 - 1) + blockVertices_->operator[](blockTriangles_->operator[](5)[vertex]).positions[0];
                                    aux.positions[1] = chunkPos_.y * CHUNK_SIZE + y + blockVertices_->operator[](blockTriangles_->operator[](5)[vertex]).positions[1];
                                    aux.positions[2] = chunkPos_.z * CHUNK_SIZE + z + blockVertices_->operator[](blockTriangles_->operator[](5)[vertex]).positions[2];
                                    aux.additionalData[0] = bNeighbor->getMaterialIndex();

                                    chunkModel->push_back(aux);

                                }

                                // Add texture to the face.
                                models::addBlockFaceTexture(*bNeighbor, *chunkModel, "faceX+");

                            }

                        }

                    }

            }

            // Read chunk data section ends.
            blocksMutex_.unlock_shared();

        }

        renderingData_.totalSize = renderingData_.vertices.size() + renderingData_.translucentVertices.size();

        return renderingData_.totalSize;

    }

    void chunk::makeEmpty() {
    
        std::unique_lock<std::shared_mutex> lock(blocksMutex_);

        needsRemesh_ = true;
        nBlocks_ = 0;
        nBlocksPlusX_ = 0;
        nBlocksMinusX_ = 0;
        nBlocksPlusY_ = 0;
        nBlocksMinusY_ = 0;
        nBlocksPlusZ_ = 0;
        nBlocksMinusZ_ = 0;

        std::memset(blocksLocalIDs_, 0, nBlocksChunk * sizeof(unsigned short));

        floodPointLightPositions_.clear();

        std::memset(neighborBlocksPlusX_, 0, nBlocksChunkEdge * sizeof(unsigned short));
        std::memset(neighborBlocksMinusX_, 0, nBlocksChunkEdge * sizeof(unsigned short));
        std::memset(neighborBlocksPlusY_, 0, nBlocksChunkEdge * sizeof(unsigned short));
        std::memset(neighborBlocksMinusY_, 0, nBlocksChunkEdge * sizeof(unsigned short));
        std::memset(neighborBlocksPlusZ_, 0, nBlocksChunkEdge * sizeof(unsigned short));
        std::memset(neighborBlocksMinusZ_, 0, nBlocksChunkEdge * sizeof(unsigned short));

        palette_.clear();
        paletteCount_.clear();

    }

    void chunk::onUnloadAsFrontier() {

        // Check for the neighbors of this recently unloaded frontier chunk
        chunkManager::onUnloadAsFrontier(vec3{ chunkPos_.x+1, chunkPos_.y, chunkPos_.z });
        chunkManager::onUnloadAsFrontier(vec3{ chunkPos_.x-1, chunkPos_.y, chunkPos_.z });
        chunkManager::onUnloadAsFrontier(vec3{ chunkPos_.x, chunkPos_.y+1, chunkPos_.z });
        chunkManager::onUnloadAsFrontier(vec3{ chunkPos_.x, chunkPos_.y-1, chunkPos_.z });
        chunkManager::onUnloadAsFrontier(vec3{ chunkPos_.x, chunkPos_.y, chunkPos_.z+1 });
        chunkManager::onUnloadAsFrontier(vec3{ chunkPos_.x, chunkPos_.y, chunkPos_.z-1 });
    
    }

    chunk::~chunk() {}

    void chunk::reset() {
    
        blockVertices_ = nullptr;
        blockTriangles_ = nullptr;
        blockNormals_ = nullptr;

        initialised_ = false;
    
    }

    // 'chunkEvent' class.

    void chunkEvent::notify(const vec2& chunkPosXZ) {

        chunkPosXZ_ = chunkPosXZ;

        event::notify();

    }

    // 'loadChunkJob' class.

    void chunkJob::setAttributes(chunk* c, chunkJobType type, atomicRecyclingPool<chunkJob>* pool, std::condition_variable* priorityNewChunkMeshesCV,
        atomicRecyclingPool<chunk>* chunkPool) {

        chunk_ = c;
        type_ = type;
        pool_ = pool;
        priorityNewChunkMeshesCV_ = priorityNewChunkMeshesCV;
        chunkPool_ = chunkPool;

    }

    void chunkJob::process() {

        if (type_ == chunkJobType::NONE) {
        
            logger::errorLog("No chunk job type was specified");
        
        }
        else if (type_ == chunkJobType::LOAD) {
        
            chunk_->makeEmpty();
            if (world::isSaved(chunk_->chunkPos())) // Load previously saved chunk.
                chunkManager::deserializeChunk(chunk_, world::loadChunk(chunk_->chunkPos()));
            else // Generate new chunk.
                worldGen::generate(*chunk_);

            chunk_->status(chunkStatus::DECORATED);

            chunkManager::renewMesh(chunk_, false);

        }
        else if (type_ == chunkJobType::ONLYREMESH) {
        
            chunkManager::renewMesh(chunk_, false);
        
        }
        else if (type_ == chunkJobType::UNLOADANDSAVE) {

            if (chunk_->modified()) {
           
                world::saveChunk(chunk_);
                chunk_->modified(false);
            
            }
            
            chunk_->status(chunkStatus::NOTLOADED);

            chunkPool_->free(*chunk_);
            
        }
        else if (type_ == chunkJobType::PRIORITYREMESH) {

            chunkManager::renewMesh(chunk_, true);

            priorityNewChunkMeshesCV_->notify_all();

        }
        else
            logger::errorLog("Unknown specified chunk job type");

        pool_->free(*this);

    }


    // 'chunkManager' class.

    bool chunkManager::initialised_ = false;
    bool chunkManager::chunkJobSystemOverloaded_ = false;
    int chunkManager::nChunksToCompute_ = 0;

    std::atomic<bool> chunkManager::clearChunksFlag_ = false;
    std::atomic<bool> chunkManager::priorityUpdatesRemaining_ = false;

    std::atomic<bool> chunkManager::waitInitialTerrainLoaded_ = true;

    std::unordered_map<vec3, chunk*> chunkManager::clientChunks_;
    std::unordered_map<vec3, chunkRenderingData>* chunkManager::chunkMeshesUpdated_ = nullptr;
    std::unordered_map<vec3, chunkRenderingData>* chunkManager::chunkMeshesWrite_ = nullptr;
    std::unordered_map<vec3, chunkRenderingData>* chunkManager::chunkMeshesRead_ = nullptr;

    std::unordered_map<vec3, chunkVBOoperation>* chunkManager::chunkVBOoperationsWrite_ = nullptr;
    std::unordered_map<vec3, chunkVBOoperation>* chunkManager::chunkVBOoperationsRead_ = nullptr;
    std::mutex chunkManager::chunkVBOoperationsMutex_;

    std::list<chunk*> chunkManager::newChunkMeshes_;
    std::list<chunk*> chunkManager::priorityNewChunkMeshes_;

    std::mutex chunkManager::managerThreadMutex_,
               chunkManager::priorityManagerThreadMutex_,
               chunkManager::priorityNewChunkMeshesMutex_,
               chunkManager::priorityUpdatesRemainingMutex_,
               chunkManager::loadingTerrainMutex_;
    std::recursive_mutex chunkManager::newChunkMeshesMutex_,
                         chunkManager::chunksMutex_;
    std::condition_variable chunkManager::managerThreadCV_,
                            chunkManager::priorityManagerThreadCV_,
                            chunkManager::priorityNewChunkMeshesCV_,
                            chunkManager::loadingTerrainCV_;
    std::condition_variable_any chunkManager::priorityUpdatesRemainingCV_;

    std::unordered_map<unsigned int, std::unordered_map<vec3, bool>> chunkManager::AIChunkAvailable_;
    std::unordered_map<unsigned int, std::unordered_map<vec3, chunk*>> chunkManager::AIagentChunks_;
    unsigned int chunkManager::selectedAIWorld_ = 0;
    bool chunkManager::originalWorldAccess_ = true;

    vec3 chunkManager::playerChunkPosCopy_;
    std::list<vec3> chunkManager::frontierChunks_;
    std::unordered_map<vec3, std::list<vec3>::iterator> chunkManager::frontierChunksSet_;
    std::list<vec3>::iterator chunkManager::frontierIt_;

    chunkManager::closestChunk chunkManager::closestChunk_;

    threadPool* chunkManager::chunkTasks_ = nullptr;
    threadPool* chunkManager::priorityChunkTasks_ = nullptr;

    atomicRecyclingPool<chunkJob>* chunkManager::loadChunkJobs_;
    atomicRecyclingPool<chunk> chunkManager::chunksPool_;

    chunkEvent chunkManager::onChunkLoad_("On chunk load");
    chunkEvent chunkManager::onChunkUnload_("On chunk unload");

    chunkVertexBuffer* chunkManager::vbo_ = nullptr;

    std::string chunkManager::openedTerrainFileName_;

    void chunkManager::init() {

        if (initialised_)
            logger::errorLog("Chunk management system was already initialised");
        else {
            
            nChunksToCompute_ = 0;

            waitInitialTerrainLoaded_ = true;

            openedTerrainFileName_ = "";

            originalWorldAccess_ = true;

            selectedAIWorld_ = 0;

            chunkMeshesUpdated_ = new std::unordered_map<vec3, chunkRenderingData>;
            chunkMeshesWrite_ = new std::unordered_map<vec3, chunkRenderingData>;
            chunkMeshesRead_ = new std::unordered_map<vec3, chunkRenderingData>;

            chunkVBOoperationsWrite_ = new std::unordered_map<vec3, chunkVBOoperation>;
            chunkVBOoperationsRead_ = new std::unordered_map<vec3, chunkVBOoperation>;

            chunkTasks_ = new threadPool(MAX_N_CHUNK_SIMULT_TASKS);
            priorityChunkTasks_ = new threadPool(MAX_N_CHUNK_SIMULT_TASKS);
            loadChunkJobs_ = new atomicRecyclingPool<chunkJob>(MAX_N_CHUNK_SIMULT_TASKS);
            loadChunkJobs_->setAllFreeOnClear(true);
            chunksPool_.setAllFreeOnClear(false);
            vbo_ = static_cast<chunkVertexBuffer*>(graphics::pVbo("chunks"));
            vbo_->bind();
            vbo_->prepareDynamic(1024 * 1024 * 1024); // 1024 MB = 1GB.

            clearChunksFlag_ = false;
            priorityUpdatesRemaining_ = false;

            initialised_ = true;

        } 

    }

    void chunkManager::updatePriorityReadChunkMeshes() {

        chunk* c = nullptr;

        std::unique_lock<std::mutex> priorityUpdatesLock(priorityUpdatesRemainingMutex_);

        for (auto it = priorityNewChunkMeshes_.begin(); it != priorityNewChunkMeshes_.end();) {

            c = *it;     
            c->lockSharedRenderingDataMutex();
            chunkRenderingData& data = c->renderingData();
            if (data.totalSize) {

                chunkMeshesUpdated_->operator[](c->chunkPos()) = data;

                chunkVBOoperationsMutex_.lock();
                chunkVBOoperationsWrite_->operator[](c->chunkPos()) = chunkVBOoperation::PUSH;
                chunkVBOoperationsMutex_.unlock();

                //logger::debugLog("Chunkpos " + std::to_string(c->chunkPos()) + " with PRIORITY data " + std::to_string(data.totalSize));

            }
            c->unlockSharedRenderingDataMutex();

            it = priorityNewChunkMeshes_.erase(it);

        }
        *chunkMeshesWrite_ = *chunkMeshesUpdated_;

    }

    void chunkManager::updateReadChunkMeshes(std::unique_lock<std::mutex>& priorityUpdatesLock) {

        chunk* c = nullptr;

        for (auto it = chunkMeshesUpdated_->begin(); it != chunkMeshesUpdated_->end();) {

            playerChunkPosCopy_ = camera::cPlayerCamera()->chunkPos();

            while (priorityUpdatesRemaining_)
                priorityUpdatesRemainingCV_.wait(priorityUpdatesLock);

            if (!chunkInRenderDistance(it->first)) {
            
                chunkVBOoperationsMutex_.lock();
                chunkVBOoperationsWrite_->operator[](it->first) = chunkVBOoperation::FREE;
                chunkVBOoperationsMutex_.unlock();

                it = chunkMeshesUpdated_->erase(it);
            
            }
            else
                it++;

        }

        newChunkMeshesMutex_.lock();
        for (auto it = newChunkMeshes_.begin(); it != newChunkMeshes_.end();) {

            while (priorityUpdatesRemaining_)
                priorityUpdatesRemainingCV_.wait(priorityUpdatesLock);

            c = *it;
            if (c->status() == chunkStatus::MESHED) {

                c->lockSharedRenderingDataMutex();
                chunkRenderingData& data = c->renderingData();
                if (data.totalSize) {
                
                    chunkMeshesUpdated_->operator[](c->chunkPos()) = data;

                    chunkVBOoperationsMutex_.lock();
                    chunkVBOoperationsWrite_->operator[](c->chunkPos()) = chunkVBOoperation::PUSH;
                    chunkVBOoperationsMutex_.unlock();

                    //logger::debugLog("Chunkpos " + std::to_string(c->chunkPos()) + " with data " + std::to_string(data.totalSize));
                
                }
                c->unlockSharedRenderingDataMutex();

                it = newChunkMeshes_.erase(it);

            }
            else
                it++;

        }
        newChunkMeshesMutex_.unlock();

        *chunkMeshesWrite_ = *chunkMeshesUpdated_; // Possible way to solve this bottleneck in the future?

    }

    void chunkManager::swapChunkMeshesBuffers() {
    
        std::unordered_map<vec3, chunkRenderingData>* aux = chunkMeshesWrite_;
        chunkMeshesWrite_ = chunkMeshesRead_;
        chunkMeshesRead_ = aux;

        chunkVBOoperationsRead_->clear(); // Rendering thread is assumed to be synchronised and waiting with the chunk management thread so this is safe.
        std::unordered_map<vec3, chunkVBOoperation>* auxChunkVBOoperations = chunkVBOoperationsWrite_;
        chunkVBOoperationsWrite_ = chunkVBOoperationsRead_;
        chunkVBOoperationsRead_ = auxChunkVBOoperations;
    
    }

    const block& chunkManager::getBlock(int posX, int posY, int posZ) {

        const block* selectedBlock;
        std::unique_lock<std::recursive_mutex> lock(chunksMutex_);


        if (game::AImodeON()) {

            if (originalWorldAccess_ || AIagentChunks_.find(selectedAIWorld_) == AIagentChunks_.cend())
                selectedBlock = &getBlockOGWorld_(posX, posY, posZ);
            else {

                std::unordered_map<vec3, chunk*>& AgentChunk = AIagentChunks_[selectedAIWorld_];
                vec3 chunkPos = getChunkCoords(posX, posY, posZ);

                if (AgentChunk.find(chunkPos) == AgentChunk.cend() || !AIChunkAvailable_[selectedAIWorld_][chunkPos])
                    selectedBlock = &getBlockOGWorld_(posX, posY, posZ);
                else
                    selectedBlock = &AgentChunk[chunkPos]->getBlock(floorMod(posX, CHUNK_SIZE), floorMod(posY, CHUNK_SIZE), floorMod(posZ, CHUNK_SIZE));

            }

        }
        else
            selectedBlock = &getBlockOGWorld_(posX, posY, posZ);

        return *selectedBlock;

    }

    std::vector<const block*> chunkManager::getBlocksBox(int x1, int y1, int z1, int x2, int y2, int z2) {

        std::vector<const block*> blocks;
        int iInc = (x1 <= x2) ? 1 : -1,
            jInc = (y1 <= y2) ? 1 : -1,
            kInc = (z1 <= z2) ? 1 : -1;
        x2 += iInc; // To make the loop stop when (i, j, k), which started at (x1, y1, z1) has iterated
        y2 += jInc; // through the box leading to (x2, y2, z2) and has also iterated said last point.
        z2 += kInc;


        std::unique_lock<std::recursive_mutex> lock(chunksMutex_);
        for (int i = x1; i != x2; i += iInc) // No merece la pena hacer caché porque las posiciones de los bloques pueden estar puestas "a mala leche" y seguir dando un peor caso.
            for (int j = y1; j != y2; j += jInc)
                for (int k = z1; k != z2; k += kInc)
                    blocks.push_back((isInWorld(i, j, k)) ? &getBlock(i, j, k) : block::emptyBlockP());

        return blocks;

    }

    bool chunkManager::isChunkInWorld(const vec3& chunkPos) {

        std::unique_lock<std::recursive_mutex> lock(chunksMutex_);

        return clientChunks_.find(chunkPos) != clientChunks_.cend();

    }

    chunkStatus chunkManager::getChunkLoadLevel(const vec3& chunkPos) {

        std::unique_lock<std::recursive_mutex> lock(chunksMutex_);

        return (clientChunks_.find(chunkPos) != clientChunks_.cend()) ? clientChunks_[chunkPos]->status() : chunkStatus::NOTLOADED;

    }

    bool chunkManager::isEmptyBlock(int posX, int posY, int posZ) {

        const block* selectedBlock;
        std::unique_lock<std::recursive_mutex> lock(chunksMutex_);
        
        if (game::AImodeON()) {

            if (originalWorldAccess_ || AIagentChunks_.find(selectedAIWorld_) == AIagentChunks_.cend())
                selectedBlock = &getBlockOGWorld_(posX, posY, posZ);
            else {

                std::unordered_map<vec3, chunk*>& AgentChunk = AIagentChunks_[selectedAIWorld_];
                vec3 chunkPos = getChunkCoords(posX, posY, posZ);

                if (AgentChunk.find(chunkPos) == AgentChunk.cend() || !AIChunkAvailable_[selectedAIWorld_][chunkPos])
                    selectedBlock = &getBlockOGWorld_(posX, posY, posZ);
                else
                    selectedBlock = &AgentChunk[chunkPos]->getBlock(floorMod(posX, CHUNK_SIZE), floorMod(posY, CHUNK_SIZE), floorMod(posZ, CHUNK_SIZE));

            }

        }
        else
            selectedBlock = &getBlockOGWorld_(posX, posY, posZ);

        return selectedBlock->isEmptyBlock();

    }

    bool chunkManager::chunkInRenderDistance(const vec3& chunkPos) {

        vec3 distancePlayer = chunkDistance(chunkPos, playerChunkPosCopy_);
        return distancePlayer.x <= nChunksToCompute_ &&
               distancePlayer.z <= nChunksToCompute_ &&
               distancePlayer.y <= yChunksRange;

    }

    bool chunkManager::chunkInLODDistance(const vec3& chunkPos, unsigned int LODlevel, bool& inBorder, blockViewDir& dirX, blockViewDir& dirY, blockViewDir& dirZ) {

        vec3 signedDistancePlayer = chunkSignedDistance(chunkPos, playerChunkPosCopy_);
        vec3 distancePlayer = abs(signedDistancePlayer);
        bool inDistance = false;
        

        switch (LODlevel) {
        
            case 1:
                inDistance = distancePlayer.x <= LODlevel1Range &&
                             distancePlayer.z <= LODlevel1Range &&
                             distancePlayer.y <= LODlevel1Range;
                inBorder = distancePlayer.x == LODlevel1Range || distancePlayer.y == LODlevel1Range || distancePlayer.z == LODlevel1Range;
                break;

            case 2: // OTRO POSIBLE ERROR EL INBORDER CON == LODLEVELXRANGE - 1 ???
                inDistance = distancePlayer.x <= LODlevel2Range &&
                             distancePlayer.z <= LODlevel2Range &&
                             distancePlayer.y <= LODlevel2Range;
                inBorder = distancePlayer.x == LODlevel2Range || distancePlayer.y == LODlevel2Range || distancePlayer.z == LODlevel2Range;
                break;

            default:
                logger::errorLog("Unsupported LOD level " + std::to_string(LODlevel));
        
        }

        dirX = (signedDistancePlayer.x == 0) ? blockViewDir::NONE : ((signedDistancePlayer.x > 0) ? blockViewDir::PLUSX : blockViewDir::NEGX);
        dirY = (signedDistancePlayer.y == 0) ? blockViewDir::NONE : ((signedDistancePlayer.y > 0) ? blockViewDir::PLUSY : blockViewDir::NEGY);
        dirZ = (signedDistancePlayer.z == 0) ? blockViewDir::NONE : ((signedDistancePlayer.z > 0) ? blockViewDir::PLUSZ : blockViewDir::NEGZ);

        return inDistance;
    }

    vec3 chunkManager::chunkDistanceToPlayer(const vec3& chunkPos) {

        const vec3& playerPos = player::getCamera().chunkPos();
        return vec3{ (float)std::abs(chunkPos.x - playerPos.x),
                     (float)std::abs(chunkPos.y - playerPos.y),
                     (float)std::abs(chunkPos.z - playerPos.z) };

    }

    vec3 chunkManager::chunkDistance(const vec3& chunkPos1, const vec3& chunkPos2) {
    
        vec3 signedDistance = chunkSignedDistance(chunkPos1, chunkPos2);
        return abs(signedDistance);
    
    }

    vec3 chunkManager::chunkSignedDistance(const vec3& chunkPos1, const vec3& chunkPos2) {

        return vec3{ chunkPos1.x - chunkPos2.x,
                     chunkPos1.y - chunkPos2.y,
                     chunkPos1.z - chunkPos2.z };

    }

    double chunkManager::distanceToPlayer(const vec3& chunkPos) {
    
        const vec3& playerPos = player::getCamera().chunkPos();
        double distanceX = playerPos.x - (chunkPos.x + 0.5) * CHUNK_SIZE;
        double distanceZ = playerPos.y - (chunkPos.y + 0.5) * CHUNK_SIZE;
        double distanceY = playerPos.z - (chunkPos.z + 0.5) * CHUNK_SIZE;
        return distanceX * distanceX + distanceZ * distanceZ + distanceY * distanceY;
    
    }

    void chunkManager::setNChunksToCompute(unsigned int nChunksToCompute) {

        engineMode mode = game::selectedEngineMode();
        if (mode == engineMode::INITLEVEL || mode == engineMode::EDITLEVEL || mode == engineMode::INITRECORD)
            nChunksToCompute_ = nChunksToCompute;
        else
            logger::errorLog("Cannot change the number of chunks to compute in the current engine mode " + std::to_string((unsigned int)mode));
    
    }

    const block& chunkManager::setBlock(int x, int y, int z, const block& blockID) {

        vec3 chunkPos = getChunkCoords(x, y, z);
        std::unique_lock<std::recursive_mutex> lock(chunksMutex_);
        if (game::AImodeON()) {
        
            if (AIagentChunks_.find(selectedAIWorld_) == AIagentChunks_.cend()) {
            
                AIagentChunks_[selectedAIWorld_] = std::unordered_map<vec3, chunk*>(); // Store here differences between the original level and the agent's copy.
                AIChunkAvailable_[selectedAIWorld_] = std::unordered_map<vec3, bool>(); // Store if the agent's copy chunk can be accessed.
            
            }
               
            std::unordered_map<vec3, chunk*>& agentWorld = AIagentChunks_[selectedAIWorld_];
            if (agentWorld.find(chunkPos) == agentWorld.cend()) {
            
                if (clientChunks_.find(chunkPos) == clientChunks_.cend())
                    logger::errorLog("There is no chunk " + std::to_string(chunkPos.x) + '|' + std::to_string(chunkPos.y) + '|' + std::to_string(chunkPos.z) +
                                     "for AI agent " + std::to_string(selectedAIWorld_));
                else {
                    
                    agentWorld[chunkPos] = new chunk(*clientChunks_[chunkPos]);
                    AIChunkAvailable_[selectedAIWorld_][chunkPos] = true;
                
                }
                
            }
            
            return agentWorld[chunkPos]->setBlock(getChunkRelCoords(x, y, z), blockID); // Chunks are not rendered in AI mode.
        
        }
        else {
        
            if (clientChunks_.find(chunkPos) == clientChunks_.cend())
                logger::errorLog("Chunk " + std::to_string(chunkPos.x) + "|" + std::to_string(chunkPos.y) + "|" + std::to_string(chunkPos.z) + " does not exist");
            else {
            
                block removedBlock = clientChunks_[chunkPos]->setBlock(getChunkRelCoords(x, y, z), blockID);

                return removedBlock;
            
            }
                 
        }

    }

    chunk* chunkManager::selectChunk(const vec3& chunkPos) {

        std::unique_lock<std::recursive_mutex> lock(chunksMutex_);

        auto it = clientChunks_.find(chunkPos);
        if (it != clientChunks_.end() && it->second->status() >= chunkStatus::DECORATED)
            return it->second;
        else
            return nullptr;

    }

    chunk* chunkManager::selectChunkByChunkPos(int x, int y, int z) {

        vec3 chunkPos = getChunkCoords(x, y, z);

        std::unique_lock<std::recursive_mutex> lock(chunksMutex_);
        auto it = clientChunks_.find(chunkPos);
        if (it != clientChunks_.end() && it->second->status() >= chunkStatus::DECORATED)
            return clientChunks_.at(chunkPos);
        else
            return nullptr;

    }

    chunk* chunkManager::selectChunkByRealPos(const vec3& pos) {

        vec3 chunkPos = getChunkCoords(pos);

        std::unique_lock<std::recursive_mutex> lock(chunksMutex_);
        auto it = clientChunks_.find(chunkPos);
        if (it != clientChunks_.end() && it->second->status() >= chunkStatus::DECORATED)
            return clientChunks_.at(chunkPos);
        else
            return nullptr;

    }

    chunk* chunkManager::neighborMinusX(const vec3& chunkPos) {

        vec3 neighborChunkPos{ chunkPos.x - 1, chunkPos.y, chunkPos.z };


        std::unique_lock<std::recursive_mutex> lock(chunksMutex_);

        if (clientChunks_.find(neighborChunkPos) != clientChunks_.end())
            return clientChunks_[neighborChunkPos];
        else
            return nullptr;

    }

    chunk* chunkManager::neighborPlusX(const vec3& chunkPos) {

        vec3 neighborChunkPos{ chunkPos.x + 1, chunkPos.y, chunkPos.z };


        std::unique_lock<std::recursive_mutex> lock(chunksMutex_);

        if (clientChunks_.find(neighborChunkPos) != clientChunks_.end())
            return clientChunks_[neighborChunkPos];
        else
            return nullptr;

    }

    chunk* chunkManager::neighborMinusY(const vec3& chunkPos) {

        vec3 neighborChunkPos{ chunkPos.x, chunkPos.y - 1, chunkPos.z };


        std::unique_lock<std::recursive_mutex> lock(chunksMutex_);

        if (clientChunks_.find(neighborChunkPos) != clientChunks_.end())
            return clientChunks_[neighborChunkPos];
        else
            return nullptr;

    }

    chunk* chunkManager::neighborPlusY(const vec3& chunkPos) {

        vec3 neighborChunkPos{ chunkPos.x, chunkPos.y + 1, chunkPos.z };


        std::unique_lock<std::recursive_mutex> lock(chunksMutex_);

        if (clientChunks_.find(neighborChunkPos) != clientChunks_.end())
            return clientChunks_[neighborChunkPos];
        else
            return nullptr;

    }

    chunk* chunkManager::neighborMinusZ(const vec3& chunkPos) {

        vec3 neighborChunkPos{ chunkPos.x, chunkPos.y, chunkPos.z - 1 };


        std::unique_lock<std::recursive_mutex> lock(chunksMutex_);

        if (clientChunks_.find(neighborChunkPos) != clientChunks_.end())
            return clientChunks_[neighborChunkPos];
        else
            return nullptr;

    }

    chunk* chunkManager::neighborPlusZ(const vec3& chunkPos) {

        vec3 neighborChunkPos{ chunkPos.x, chunkPos.y, chunkPos.z + 1 };


        std::unique_lock<std::recursive_mutex> lock(chunksMutex_);

        if (clientChunks_.find(neighborChunkPos) != clientChunks_.end())
            return clientChunks_[neighborChunkPos];
        else
            return nullptr;

    }

    void chunkManager::waitInitialTerrainLoaded() {

        {

            std::unique_lock<std::mutex> lock(loadingTerrainMutex_);
            while(waitInitialTerrainLoaded_)
                loadingTerrainCV_.wait(lock);

        }

    }

    void chunkManager::unloadFrontierChunk(const vec3& chunkPos) {

        std::unique_lock<std::recursive_mutex> lock(chunksMutex_);
        
        auto it = clientChunks_.find(chunkPos);

        if (it == clientChunks_.end())
            logger::errorLog("Chunk at " + std::to_string(chunkPos) + " is not registered");
        else {

            chunk* unloadedChunk = it->second;

            // Check if the chunk's neighbors become frontier chunks after it is unloaded.
            unloadedChunk->onUnloadAsFrontier();

            frontierChunks_.erase(frontierChunksSet_.at(chunkPos));
            frontierChunksSet_.erase(chunkPos);

            clientChunks_.erase(chunkPos);

            onChunkUnload_.notify(chunkPos.x, chunkPos.z);
            onChunkUnload_.notify(chunkPos.x + 1, chunkPos.z);
            onChunkUnload_.notify(chunkPos.x - 1, chunkPos.z);
            onChunkUnload_.notify(chunkPos.x, chunkPos.z + 1);
            onChunkUnload_.notify(chunkPos.x, chunkPos.z - 1);

            issueChunkMeshJob(unloadedChunk, chunkJobType::UNLOADANDSAVE);

        }
        
    }

    void chunkManager::onUnloadAsFrontier(const vec3& chunkPos) {

        if (!frontierChunksSet_.contains(chunkPos) && clientChunks_.contains(chunkPos))
            frontierChunksSet_[chunkPos] = frontierChunks_.insert(frontierChunks_.end(), chunkPos);
    
    }

    void chunkManager::addFrontier(chunk* chunk) {
    
        const vec3& chunkPos = chunk->chunkPos();
        if (!frontierChunksSet_.contains(chunkPos))
            frontierChunksSet_[chunkPos] = frontierChunks_.insert(frontierChunks_.end(), chunkPos);
    
    }

    void chunkManager::renewMesh(chunk* chunk, bool isPriorityUpdate) {
    
        unsigned int vertexSize = chunk->renewMesh();
        chunk->status(chunkStatus::MESHED);

        if (isPriorityUpdate) {

            priorityNewChunkMeshesMutex_.lock();
            priorityNewChunkMeshes_.push_back(chunk);
            priorityNewChunkMeshesMutex_.unlock();

        }
        else if (vertexSize) {

            newChunkMeshesMutex_.lock();
            newChunkMeshes_.push_back(chunk);
            newChunkMeshesMutex_.unlock();

        }

    }

    void chunkManager::renewMesh(const vec3& chunkPos, bool isPriorityUpdate) {
    
        chunksMutex_.lock();

        auto it = clientChunks_.find(chunkPos);
        if (it == clientChunks_.cend()) {

            chunksMutex_.unlock();

            logger::errorLog("The chunk " + std::to_string(chunkPos.x) + "|" + std::to_string(chunkPos.y) + "|"
                + std::to_string(chunkPos.z) + " is not registered");
        
        }
        else {
            
            chunksMutex_.unlock();

            renewMesh(it->second, isPriorityUpdate);

        }
            
    }

    void chunkManager::manageChunks() {

        {
            // NEXT. LOS JOBS DE NEIGHBOR QUE SE TIENE QUE ACTUALIZAR CON PRIORITY DEBEN IR EN UN MISMO JOB SINO DA PROBLEMAS.

            std::unique_lock<std::mutex> lock(managerThreadMutex_),
                                         priorityUpdatesLock(priorityUpdatesRemainingMutex_);

            /*
            Initialise general parts of the engine related to maintaining
            an "infinite" world type.
            */

            // First of all, load the chunk where the player is in.
            bool continueCreatingChunks = false;
            vec3 chunkPos = vec3Zero;
            unsigned int nIterations = 0;
            const unsigned int defaultMaxIterations = 128;
            unsigned int maxIterations = defaultMaxIterations;
            while (game::threadsExecute[2]) {

                continueCreatingChunks = false;
                
                ensureChunkIfVisible(playerChunkPosCopy_.x, playerChunkPosCopy_.y, playerChunkPosCopy_.z); // NEXT. ASEGURARSE DE QUE ESTE CHUNK SEA EL DEL JUGADOR YA POSICIONADO BIEN TRAS CARGA DE MUNDO.

                do {

                    maxIterations = chunkTasks_->size() < 10000 ? defaultMaxIterations : 1;

                    // Load new chunks that are inside render distance if necessary.
                    // Mark frontier chunks that are no longer frontier.
                    frontierChunks_.sort(closestChunk_);
                    continueCreatingChunks = false;
                    nIterations = 0;
                    for (frontierIt_ = frontierChunks_.begin(); frontierIt_ != frontierChunks_.end() && nIterations < maxIterations;) {

                        playerChunkPosCopy_ = camera::cPlayerCamera()->chunkPos();
                        chunkPos = *frontierIt_;

                        while (priorityUpdatesRemaining_)
                            priorityUpdatesRemainingCV_.wait(priorityUpdatesLock);

                        if (!chunkInRenderDistance(chunkPos)) {
                        
                            frontierIt_++;
                            unloadFrontierChunk(chunkPos);

                            chunkVBOoperationsMutex_.lock();
                            chunkVBOoperationsWrite_->operator[](chunkPos) = chunkVBOoperation::FREE;
                            chunkVBOoperationsMutex_.unlock();
                        
                        }
                        else if 
                            (ensureChunkIfVisible(chunkPos.x + 1, chunkPos.y, chunkPos.z) +
                            ensureChunkIfVisible(chunkPos.x - 1, chunkPos.y, chunkPos.z) +
                            ensureChunkIfVisible(chunkPos.x, chunkPos.y + 1, chunkPos.z) +
                            ensureChunkIfVisible(chunkPos.x, chunkPos.y - 1, chunkPos.z) +
                            ensureChunkIfVisible(chunkPos.x, chunkPos.y, chunkPos.z + 1) +
                            ensureChunkIfVisible(chunkPos.x, chunkPos.y, chunkPos.z - 1) == 6) {

                            chunksMutex_.lock();
                            chunk* c = clientChunks_[*frontierIt_];
                            chunksMutex_.unlock();
                            if (c->renderingData().vertices.size()) {

                                newChunkMeshesMutex_.lock();

                                newChunkMeshes_.push_back(c);

                                newChunkMeshesMutex_.unlock();

                            }

                            continueCreatingChunks = ++nIterations < maxIterations;

                            frontierChunksSet_.erase(*frontierIt_);
                            frontierIt_ = frontierChunks_.erase(frontierIt_);

                        }
                        else
                            frontierIt_++;
                            
                    }

                } while (continueCreatingChunks);

                // Do not attempt to synchronize with the rendering thread if the initial
                // preparations for loading a level are not completed yet.
                if (waitInitialTerrainLoaded_) {
                
                    waitInitialTerrainLoaded_ = false;
                    loadingTerrainCV_.notify_one();
                
                }

                // Sync with the rendering thread in order to pass it the most updated
                // version of the chunks' meshes.
                updateReadChunkMeshes(priorityUpdatesLock);
                managerThreadCV_.wait(lock);

                {
                
                    using namespace std::chrono_literals;
                    std::this_thread::sleep_for(1ms);
                
                }

            }

        }

    }

    void chunkManager::manageChunkPriorityUpdates() {

        {

            std::unique_lock<std::mutex> lock(priorityManagerThreadMutex_),
                                         lockNewChunksMeshes(priorityNewChunkMeshesMutex_);

            priorityNewChunkMeshesCV_.wait(lockNewChunksMeshes);
            while (game::threadsExecute[2]) {

                priorityUpdatesRemaining_ = true;

                // Sync with the rendering thread in order to pass it the most updated
                // version of the chunks' meshes.
                updatePriorityReadChunkMeshes();
                priorityManagerThreadCV_.wait(lock);

                priorityUpdatesRemaining_ = false;
                priorityUpdatesRemainingCV_.notify_all();

                priorityNewChunkMeshesCV_.wait(lockNewChunksMeshes);

                {

                    using namespace std::chrono_literals;
                    std::this_thread::sleep_for(1ms);

                }

            }

        }

    }

    // TODO. CORREGIR
    void chunkManager::generateAIWorld(const std::string& path) {

        if (game::AImodeON()) {

            chunk* selectedChunk = nullptr;
            vec3 chunkPos;

            timer t;
            t.start();
            if (path.empty()) {

                worldGen::prepareGen();

                for (chunkPos.y = -yChunksRange; chunkPos.y < yChunksRange; chunkPos.y++)
                    for (chunkPos.x = -nChunksToCompute_; chunkPos.x < nChunksToCompute_; chunkPos.x++)
                        for (chunkPos.z = -nChunksToCompute_; chunkPos.z < nChunksToCompute_; chunkPos.z++) {


                            //selectedChunk = chunkManager::createChunk(false, chunkPos);
                            clientChunks_.insert_or_assign(chunkPos, selectedChunk);

                        }

            }

            t.finish();
            logger::debugLog("Generated AI world on " + std::to_string(t.getDurationMs()) + "ms");

        }
        else
            logger::errorLog("Chunk manager's AI mode must be turned on when generating a world for AI testing/training.");

    }

    void chunkManager::selectAIworld(unsigned int individualID) {

        if (game::AImodeON()) {

            originalWorldAccess_ = false;
            selectedAIWorld_ = individualID;

        }
        else
            logger::errorLog("AI mode needs to be enabled to select an AI agent world");

    }

    void chunkManager::selectOriginalWorld() {

        if (game::AImodeON())
            originalWorldAccess_ = true;
        else
            logger::errorLog("AI mode needs to be enabled to select an AI agent world");

    }

    void chunkManager::resetAIChunks() {
    
        timer t;

        t.start();
        for (auto it = AIChunkAvailable_.begin(); it != AIChunkAvailable_.end(); it++)
            for (auto it2 = it->second.begin(); it2 != it->second.end(); it2++)
                it2->second = false;
        t.finish();

        logger::debugLog("AI chunks copy reset done in " + std::to_string(t.getDurationMs()));
    
    }

    void chunkManager::openedTerrainFileName(const std::string& newFilename) {

        if (game::selectedEngineMode() == VoxelEng::engineMode::EDITLEVEL)
            logger::errorLog("Cannot change the opened terrain file name while in a level");
        else
            openedTerrainFileName_ = newFilename;

    }

    bool chunkManager::ensureChunkIfVisible(const vec3& chunkPos) {
       
        bool chunkInDistance = chunkInRenderDistance(chunkPos);

        chunksMutex_.lock();
        if (chunkInDistance) { 

            if (clientChunks_.contains(chunkPos)) {

                chunksMutex_.unlock();
                return true;

            }
            else {
            
                chunksMutex_.unlock();
                return loadChunk(chunkPos) != nullptr;
            
            }

        }
        else {
        
            chunksMutex_.unlock();
            return false;
        
        }
        
    }

    std::string chunkManager::serializeChunk(chunk* c) {

        timer t;
        t.start();

        c->blockDataMutex().lock_shared();

        // Save local ID data (including duplicated data about neighbors).
        std::string data((const char*)c->blocks(), sizeof(unsigned short) * nBlocksChunk);

        data += '@';
        data.append((const char*)c->neighborBlocksPlusX(), sizeof(unsigned short) * nBlocksChunkEdge);

        data += '@';
        data.append((const char*)c->neighborBlocksMinusX(), sizeof(unsigned short) * nBlocksChunkEdge);

        data += '@';
        data.append((const char*)c->neighborBlocksPlusY(), sizeof(unsigned short) * nBlocksChunkEdge);

        data += '@';
        data.append((const char*)c->neighborBlocksMinusY(), sizeof(unsigned short) * nBlocksChunkEdge);

        data += '@';
        data.append((const char*)c->neighborBlocksPlusZ(), sizeof(unsigned short) * nBlocksChunkEdge);

        data += '@';
        data.append((const char*)c->neighborBlocksMinusZ(), sizeof(unsigned short) * nBlocksChunkEdge);

        data += '@';

        // Save palette data.
        const palette<unsigned short, unsigned int>& chunkPalette = c->getPalette();
        for (auto it = chunkPalette.cbegin(); it != chunkPalette.cend(); it++)
            data += std::to_string(it->first) + '|' + block::getBlockC(it->second).name() + '|';

        data += '@';

        const std::unordered_map<unsigned short, unsigned short>& chunkPaletteCount = c->getPaletteCount();
        for (auto it = chunkPaletteCount.cbegin(); it != chunkPaletteCount.cend(); it++)
            data += std::to_string(it->first) + '|' + std::to_string(it->second) + '|';

        data += '@';

        const std::unordered_set<unsigned short>& chunkFreeLocalIDs = c->getFreeLocalIDs();
        for (auto it = chunkFreeLocalIDs.cbegin(); it != chunkFreeLocalIDs.cend(); it++)
            data += std::to_string(*it) + '|';

        data += '@';

        const std::unordered_set<vec3>& floodPointLightPositions = c->getFloodPointLightPositions();
        for (auto it = floodPointLightPositions.cbegin(); it != floodPointLightPositions.cend(); it++)
            data += std::to_string((int)it->x) + '|' + std::to_string((int)it->y) + '|' + std::to_string((int)it->z) + '|';

        data += '@';

        data += std::to_string(c->nBlocks()) + '|' + std::to_string(c->nBlocksPlusX()) + '|' + std::to_string(c->nBlocksMinusX()) + '|' + std::to_string(c->nBlocksPlusY()) + '|' + std::to_string(c->nBlocksMinusY()) + '|' + std::to_string(c->nBlocksPlusZ()) + '|' + std::to_string(c->nBlocksMinusZ()) + '|';

        c->blockDataMutex().unlock_shared();

        t.finish();
        logger::debugLog("Time for saving chunks is " + std::to_string(t.getDurationMs()));

        return data;
    
    }

    void chunkManager::deserializeChunk(chunk* chunk, const std::string& data) {
    
        std::string word;
        char c = 0;
        unsigned int index = 0,
            nBytes = data.size();
        const char* dataBegin = &data[index];
        unsigned short localID = 0;
        unsigned int globalID = 0;
        unsigned short count = 0;
        unsigned int x = 0;
        unsigned int y = 0;
        unsigned int z = 0;

        chunk->blockDataMutex().lock();
        std::memcpy(chunk->blocks(), dataBegin, sizeof(unsigned short) * nBlocksChunk);
        index += sizeof(unsigned short) * nBlocksChunk + 1;

        std::memcpy(chunk->neighborBlocksPlusX(), &data[index], sizeof(unsigned short) * nBlocksChunkEdge);
        index += sizeof(unsigned short) * nBlocksChunkEdge + 1;

        std::memcpy(chunk->neighborBlocksMinusX(), &data[index], sizeof(unsigned short) * nBlocksChunkEdge);
        index += sizeof(unsigned short) * nBlocksChunkEdge + 1;

        std::memcpy(chunk->neighborBlocksPlusY(), &data[index], sizeof(unsigned short) * nBlocksChunkEdge);
        index += sizeof(unsigned short) * nBlocksChunkEdge + 1;

        std::memcpy(chunk->neighborBlocksMinusY(), &data[index], sizeof(unsigned short) * nBlocksChunkEdge);
        index += sizeof(unsigned short) * nBlocksChunkEdge + 1;

        std::memcpy(chunk->neighborBlocksPlusZ(), &data[index], sizeof(unsigned short) * nBlocksChunkEdge);
        index += sizeof(unsigned short) * nBlocksChunkEdge + 1;

        std::memcpy(chunk->neighborBlocksMinusZ(), &data[index], sizeof(unsigned short) * nBlocksChunkEdge);
        index += sizeof(unsigned short) * nBlocksChunkEdge + 1;

        palette<unsigned short, unsigned int>& chunkPalette = chunk->getPalette();
        c = data[index];
        while (c != '@') {

            while (c != '|') {

                word += c;

                c = data[++index];

            }

            localID = sto<unsigned short>(word);
            word.clear();

            c = data[++index];

            while (c != '|') {

                word += c;

                c = data[++index];

            }

            globalID = block::getBlockC(word).intID();
            word.clear();

            chunkPalette.insert(localID, globalID);

            c = data[++index];

        }

        std::unordered_map<unsigned short, unsigned short>& chunkPaletteCount = chunk->getPaletteCount();
        c = data[++index]; // Skip the '@' delimiter character.
        while (c != '@') {

            while (c != '|') {

                word += c;

                c = data[++index];

            }

            localID = sto<unsigned short>(word);
            word.clear();

            c = data[++index];

            while (c != '|') {

                word += c;
                c = data[++index];

            }

            count = sto<unsigned short>(word);
            word.clear();

            chunkPaletteCount[localID] = count;

            c = data[++index];

        }

        std::unordered_set<unsigned short>& chunkFreeLocalIDs = chunk->getFreeLocalIDs();
        c = data[++index]; // Skip the '@' delimiter character.
        while (c != '@') {

            while (c != '|') {

                word += c;

                c = data[++index];

            }

            localID = sto<unsigned short>(word);
            chunkFreeLocalIDs.insert(localID);
            word.clear();

            c = data[++index];

        }

        std::unordered_set<vec3>& floodPointLightPositions = chunk->getFloodPointLightPositions();
        c = data[++index]; // Skip the '@' delimiter character.
        while (c != '@') {

            for (int i = 0; i < 3; i++) {
            
                while (c != '|') {

                    word += c;

                    c = data[++index];

                }

                switch (i) 
                {
                    case 0:
                        x = sto<unsigned int>(word);
                        break;
                    case 1:
                        y = sto<unsigned int>(word);
                        break;
                    case 2:
                        z = sto<unsigned int>(word);
                        break;
                    default:
                        throw std::runtime_error("This is not possible. Something has wrong horribly wrong when loading chunk from disk");
                }

                word.clear();
                c = data[++index];
                
            }
            
            floodPointLightPositions.insert(vec3{x,y,z});

        }

        chunk->blockDataMutex().unlock();

        c = data[++index]; // Skip the '@' delimiter character.
        unsigned int state = 0;
        while (index < nBytes) {

            while (c != '|') {

                word += c;

                c = data[++index];

            }

            switch (state) {

            case 0:
                chunk->nBlocks(sto<unsigned short>(word));
                break;
            case 1:
                chunk->nBlocksPlusX(sto<unsigned short>(word));
                break;
            case 2:
                chunk->nBlocksMinusX(sto<unsigned short>(word));
                break;
            case 3:
                chunk->nBlocksPlusY(sto<unsigned short>(word));
                break;
            case 4:
                chunk->nBlocksMinusY(sto<unsigned short>(word));
                break;
            case 5:
                chunk->nBlocksPlusZ(sto<unsigned short>(word));
                break;
            case 6:
                chunk->nBlocksMinusZ(sto<unsigned short>(word));
                break;

            }

            word.clear();
            c = data[++index];
            state++;

        }

        chunk->needsRemesh(true); // TODO. EL BUG ES QUE SI MODIFICO UN BLOCK EN UN BORDE, TAMBIEN HAY QUE GUARDAR EL CHUNK VECINO QUE LE HACE FRONTERA.
    
    }

    chunk* chunkManager::loadChunk(const vec3& chunkPos) {

        chunk* c = &chunksPool_.get();

        c->chunkPos(chunkPos);

        chunksMutex_.lock();
        clientChunks_[chunkPos] = c;
        chunksMutex_.unlock();

        addFrontier(c);

        onChunkLoad_.notify(chunkPos.x, chunkPos.z);
        onChunkLoad_.notify(chunkPos.x + 1, chunkPos.z);
        onChunkLoad_.notify(chunkPos.x - 1, chunkPos.z);
        onChunkLoad_.notify(chunkPos.x, chunkPos.z + 1);
        onChunkLoad_.notify(chunkPos.x, chunkPos.z - 1);

        // Submit async task to load the chunk either from disk or by generating it.
        issueChunkMeshJob(c, chunkJobType::LOAD);

        return c;

    }

    void chunkManager::issueChunkMeshJob(chunk* c, chunkJobType type) {
    
        chunkJob* aJob = &loadChunkJobs_->get();
        aJob->setAttributes(c, type, loadChunkJobs_, &priorityNewChunkMeshesCV_, &chunksPool_);

        if (type == chunkJobType::PRIORITYREMESH) {
        
            c->needsRemesh(true);
            priorityChunkTasks_->submitJob(aJob);
        
        }
        else
            chunkTasks_->submitJob(aJob);
    
    }

    void chunkManager::clear() { 

        if (chunkTasks_)
            chunkTasks_->awaitNoJobs();

        if (priorityChunkTasks_)
            priorityChunkTasks_->awaitNoJobs();

        chunksMutex_.lock();
        for (auto it = clientChunks_.begin(); it != clientChunks_.end(); it++)
            if (it->second)
                delete it->second;
        clientChunks_.clear();
        chunksMutex_.unlock();

        if (chunkMeshesUpdated_)
            chunkMeshesUpdated_->clear();

        if (chunkMeshesWrite_)
            chunkMeshesWrite_->clear();

        if (chunkMeshesRead_)
            chunkMeshesRead_->clear();

        chunkVBOoperationsMutex_.lock();
        if (chunkVBOoperationsRead_)
            chunkVBOoperationsRead_->clear();

        if (chunkVBOoperationsWrite_)
            chunkVBOoperationsWrite_->clear();
        chunkVBOoperationsMutex_.unlock();

        chunksPool_.clear();

        priorityNewChunkMeshesMutex_.lock();
        priorityNewChunkMeshes_.clear();
        priorityNewChunkMeshesMutex_.unlock();

        newChunkMeshesMutex_.lock();
        newChunkMeshes_.clear();
        newChunkMeshesMutex_.unlock();

        frontierChunks_.clear();
        frontierChunksSet_.clear();

        for (auto it = AIagentChunks_.cbegin(); it != AIagentChunks_.cend(); it++)
            for (auto itChunks = it->second.cbegin(); itChunks != it->second.cend(); itChunks++)
                delete itChunks->second;
        AIagentChunks_.clear();

    }

    void chunkManager::reset() {

        if (chunkTasks_) {

            chunkTasks_->shutdown();
            chunkTasks_->awaitTermination();

            delete chunkTasks_;
            chunkTasks_ = nullptr;

        }

        if (priorityChunkTasks_) {

            priorityChunkTasks_->shutdown();
            priorityChunkTasks_->awaitTermination();

            delete priorityChunkTasks_;
            priorityChunkTasks_ = nullptr;

        }

        if (loadChunkJobs_) {

            loadChunkJobs_->clear();

            delete loadChunkJobs_;
            loadChunkJobs_ = nullptr;

        }

        clear();

        if (chunkMeshesUpdated_) {

            delete chunkMeshesUpdated_;
            chunkMeshesUpdated_ = nullptr;

        }

        if (chunkMeshesWrite_) {

            delete chunkMeshesWrite_;
            chunkMeshesWrite_ = nullptr;

        }

        if (chunkMeshesRead_) {

            delete chunkMeshesRead_;
            chunkMeshesRead_ = nullptr;

        }

        chunkVBOoperationsMutex_.lock();
        if (chunkVBOoperationsRead_) {

            delete chunkVBOoperationsRead_;
            chunkVBOoperationsRead_ = nullptr;

        }

        if (chunkVBOoperationsWrite_) {

            delete chunkVBOoperationsWrite_;
            chunkVBOoperationsWrite_ = nullptr;

        }
        chunkVBOoperationsMutex_.unlock();

        initialised_ = false;
        
    }

    const block& chunkManager::getBlockOGWorld_(int posX, int posY, int posZ) {
    
        vec3 chunkPos = getChunkCoords(posX, posY, posZ);

        if (clientChunks_.find(chunkPos) == clientChunks_.end())
            logger::errorLog("Chunk " + std::to_string(chunkPos.x) + "|" + std::to_string(chunkPos.y) + "|" + std::to_string(chunkPos.z) + " does not exist");
        else
            return clientChunks_[chunkPos]->getBlock(floorMod(posX, CHUNK_SIZE), floorMod(posY, CHUNK_SIZE), floorMod(posZ, CHUNK_SIZE));
    
    }

}
