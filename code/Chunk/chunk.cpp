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

#include <camera.h>
#include <player.h>
#include <input.h>
#include <gui.h>
#include <game.h>
#include <Chunk/chunkDefinitions.h>
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
#include <World/WorldGen/worldGen.h>


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
    : blocksLocalIDs_(16, 16, 16, 1, 0),
      modified_(false),
      nOpaqueBlocks_(0),
      nOpaqueBlocksPlusX_(0),
      nOpaqueBlocksMinusX_(0),
      nOpaqueBlocksPlusY_(0),
      nOpaqueBlocksMinusY_(0),
      nOpaqueBlocksPlusZ_(0),
      nOpaqueBlocksMinusZ_(0),
      nTotalBlocks_(0),
      nTotalBlocksPlusX_(0),
      nTotalBlocksMinusX_(0),
      nTotalBlocksPlusY_(0),
      nTotalBlocksMinusY_(0),
      nTotalBlocksPlusZ_(0),
      nTotalBlocksMinusZ_(0),
      needsRemesh_(false), 
      loadedFromDisk_(false),
      loadLevel_(chunkStatus::NOTLOADED),
      chunkPos_(vec3Zero) {

        std::memset(blockLightColor_, 0, nBlocksChunk * sizeof(basicVec4));
        std::memset(blockLightLevel_, -1, nBlocksChunk * sizeof(char));
    
    }
   
    chunk::chunk(bool empty, const vec3& chunkPos)
    : blocksLocalIDs_(16, 16, 16, 1, 0),
      modified_(false),
      nOpaqueBlocks_(0),
      nOpaqueBlocksPlusX_(0),
      nOpaqueBlocksMinusX_(0),
      nOpaqueBlocksPlusY_(0),
      nOpaqueBlocksMinusY_(0),
      nOpaqueBlocksPlusZ_(0),
      nOpaqueBlocksMinusZ_(0),
      nTotalBlocks_(0),
      nTotalBlocksPlusX_(0),
      nTotalBlocksMinusX_(0),
      nTotalBlocksPlusY_(0),
      nTotalBlocksMinusY_(0),
      nTotalBlocksPlusZ_(0),
      nTotalBlocksMinusZ_(0),
      loadLevel_(chunkStatus::NOTLOADED),
      needsRemesh_(false),
      loadedFromDisk_(false),
      chunkPos_(vec3Zero) {

        std::memset(blockLightColor_, 0, nBlocksChunk * sizeof(basicVec4));
        std::memset(blockLightLevel_, -1, nBlocksChunk * sizeof(char));
        
        if (!empty)
            worldGen::generate(*this); // This can only call to setBlock to modify the chunk and that method already takes care of 'blocksMutex_'.
        
    }

    chunk::chunk(chunk& c)
    : blocksLocalIDs_(c.blocksLocalIDs_),
      modified_(c.modified_),
      nOpaqueBlocks_(c.nOpaqueBlocks_.load()),
      nOpaqueBlocksPlusX_(c.nOpaqueBlocksPlusX_.load()),
      nOpaqueBlocksMinusX_(c.nOpaqueBlocksMinusX_.load()),
      nOpaqueBlocksPlusY_(c.nOpaqueBlocksPlusY_.load()),
      nOpaqueBlocksMinusY_(c.nOpaqueBlocksMinusY_.load()),
      nOpaqueBlocksPlusZ_(c.nOpaqueBlocksPlusZ_.load()),
      nOpaqueBlocksMinusZ_(c.nOpaqueBlocksMinusZ_.load()),
      nTotalBlocks_(c.nTotalBlocks_.load()),
      nTotalBlocksPlusX_(c.nTotalBlocksPlusX_.load()),
      nTotalBlocksMinusX_(c.nTotalBlocksMinusX_.load()),
      nTotalBlocksPlusY_(c.nTotalBlocksPlusY_.load()),
      nTotalBlocksMinusY_(c.nTotalBlocksMinusY_.load()),
      nTotalBlocksPlusZ_(c.nTotalBlocksPlusZ_.load()),
      nTotalBlocksMinusZ_(c.nTotalBlocksMinusZ_.load()),
      loadLevel_(c.loadLevel_.load()),
      needsRemesh_(c.needsRemesh_.load()),
      loadedFromDisk_(c.loadedFromDisk_.load()),
      chunkPos_(c.chunkPos_) {

        c.blocksMutex_.lock_shared();

        renderingData_.globalChunkPos = c.renderingData_.globalChunkPos;

        palette_ = c.palette_;
        paletteCount_ = c.paletteCount_;
        freeLocalIDs_ = c.freeLocalIDs_;

        std::memcpy(blockLightColor_, c.blockLightColor_, nBlocksChunk * sizeof(basicVec4));
        std::memcpy(blockLightLevel_, c.blockLightLevel_, nBlocksChunk * sizeof(char));

        renderingData_.vertices = c.renderingData_.vertices;

        c.blocksMutex_.unlock_shared();

    }

    const block& chunk::getBlock(GLbyte x, GLbyte y, GLbyte z, bool lock) {

        //if(lock)
            //blocksMutex_.lock_shared();

        unsigned int localID = blocksLocalIDs_[x][y][z];

        return localID ? block::getBlockC(palette_.getT2(localID)) : block::emptyBlock();

        //if (lock)
            //blocksMutex_.unlock_shared();

    }

    const block& chunk::getNeighborBlock(GLbyte firstIndex, GLbyte secondIndex, blockViewDir neighbor) {
    
        unsigned int localID = 0;

        switch (neighbor) {

        case blockViewDir::PLUSX:
            localID = blocksLocalIDs_[CHUNK_SIZE][firstIndex][secondIndex];
            break;

        case blockViewDir::NEGX:
            localID = blocksLocalIDs_[-1][firstIndex][secondIndex];
            break;

        case blockViewDir::PLUSY:
            localID = blocksLocalIDs_[firstIndex][CHUNK_SIZE][secondIndex];
            break;

        case blockViewDir::NEGY:
            localID = blocksLocalIDs_[firstIndex][-1][secondIndex];
            break;

        case blockViewDir::PLUSZ:
            localID = blocksLocalIDs_[firstIndex][secondIndex][CHUNK_SIZE];
            break;

        case blockViewDir::NEGZ:
            localID = blocksLocalIDs_[firstIndex][secondIndex][-1];
            break;

        }

        return localID ? block::getBlockC(palette_.getT2(localID)) : block::emptyBlock();
    
    }

    bool chunk::isEmptyNeighborBlock(unsigned int firstIndex, unsigned int secondIndex, blockViewDir neighbor) {

        switch (neighbor) {
        
            case blockViewDir::PLUSX:
                return blocksLocalIDs_[CHUNK_SIZE][firstIndex][secondIndex] == 0;
                break;

            case blockViewDir::NEGX:
                return blocksLocalIDs_[-1][firstIndex][secondIndex] == 0;
                break;

            case blockViewDir::PLUSY:
                return blocksLocalIDs_[firstIndex][CHUNK_SIZE][secondIndex] == 0;
                break;

            case blockViewDir::NEGY:
                return blocksLocalIDs_[firstIndex][-1][secondIndex] == 0;
                break;

            case blockViewDir::PLUSZ:
                return blocksLocalIDs_[firstIndex][secondIndex][CHUNK_SIZE] == 0;
                break;

            case blockViewDir::NEGZ:
                return blocksLocalIDs_[firstIndex][secondIndex][-1] == 0;
                break;
        
        }
    
    }

    const block& chunk::setBlock(GLbyte x, GLbyte y, GLbyte z, const block& b, bool modification) {

        unsigned short& actualLocalID = blocksLocalIDs_[x][y][z];
        unsigned short oldLocalID = actualLocalID;
        unsigned int oldGlobalID = actualLocalID ? palette_.getT2(actualLocalID) : 0;
        const block& oldB = block::getBlockC(oldGlobalID);
 
        placeNewBlock(actualLocalID, b);

        bool blockWasModified = oldLocalID != actualLocalID;

        needsRemesh_ = needsRemesh_ || blockWasModified;

        modified_ = modified_ || (blockWasModified && modification);

        if (!oldLocalID && actualLocalID)
            nTotalBlocks_++;
        else if (oldLocalID && !actualLocalID)
            nTotalBlocks_--;
        nOpaqueBlocks_ += (b.opacity() == blockOpacity::OPAQUEBLOCK) - (oldB.opacity() == blockOpacity::OPAQUEBLOCK);

        // Update block light information.
       
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

    void chunk::setBlockNeighbor(unsigned int firstIndex, unsigned int secondIndex, blockViewDir neighbor, const block& b, bool modification) {

        unsigned short oldLocalID = 0;
        bool blockWasModified = false;
        
        if (neighbor == blockViewDir::NONE)
            logger::errorLog("No block view direction was specified");
        else if (neighbor == blockViewDir::PLUSX) {

            // TODO. REFACTOR THIS INTO A METHOD.
            unsigned short& actualLocalID = blocksLocalIDs_[CHUNK_SIZE][firstIndex][secondIndex];
            oldLocalID = actualLocalID;
            unsigned short oldLocalID = actualLocalID;
            unsigned int oldGlobalID = actualLocalID ? palette_.getT2(actualLocalID) : 0;
            const block& oldB = block::getBlockC(oldGlobalID);
            placeNewBlock(actualLocalID, b);
            blockWasModified = oldLocalID != actualLocalID;
            needsRemesh_ = needsRemesh_ || blockWasModified && blocksLocalIDs_[CHUNK_SIZE_LIMIT][firstIndex][secondIndex];
            if (!oldLocalID && actualLocalID)
                nTotalBlocksPlusX_++;
            else if (oldLocalID && !actualLocalID)
                nTotalBlocksPlusX_--;
            nOpaqueBlocksPlusX_ += (b.opacity() == blockOpacity::OPAQUEBLOCK) - (oldB.opacity() == blockOpacity::OPAQUEBLOCK);

        }
        else if (neighbor == blockViewDir::NEGX) {

            unsigned short& actualLocalID = blocksLocalIDs_[-1][firstIndex][secondIndex];
            oldLocalID = actualLocalID;
            unsigned short oldLocalID = actualLocalID;
            unsigned int oldGlobalID = actualLocalID ? palette_.getT2(actualLocalID) : 0;
            const block& oldB = block::getBlockC(oldGlobalID);
            placeNewBlock(actualLocalID, b);
            blockWasModified = oldLocalID != actualLocalID;
            needsRemesh_ = needsRemesh_ || blockWasModified && blocksLocalIDs_[0][firstIndex][secondIndex];
            if (!oldLocalID && actualLocalID)
                nTotalBlocksMinusX_++;
            else if (oldLocalID && !actualLocalID)
                nTotalBlocksMinusX_--;
            nOpaqueBlocksMinusX_ += (b.opacity() == blockOpacity::OPAQUEBLOCK) - (oldB.opacity() == blockOpacity::OPAQUEBLOCK);
            
        }
        else if (neighbor == blockViewDir::PLUSY) {

            unsigned short& actualLocalID = blocksLocalIDs_[firstIndex][CHUNK_SIZE][secondIndex];
            oldLocalID = actualLocalID;
            unsigned short oldLocalID = actualLocalID;
            unsigned int oldGlobalID = actualLocalID ? palette_.getT2(actualLocalID) : 0;
            const block& oldB = block::getBlockC(oldGlobalID);
            placeNewBlock(actualLocalID, b);
            blockWasModified = oldLocalID != actualLocalID;
            needsRemesh_ = needsRemesh_ || blockWasModified && blocksLocalIDs_[firstIndex][CHUNK_SIZE_LIMIT][secondIndex];
            if (!oldLocalID && actualLocalID)
                nTotalBlocksPlusY_++;
            else if (oldLocalID && !actualLocalID)
                nTotalBlocksPlusY_--;
            nOpaqueBlocksPlusY_ += (b.opacity() == blockOpacity::OPAQUEBLOCK) - (oldB.opacity() == blockOpacity::OPAQUEBLOCK);

        }
        else if (neighbor == blockViewDir::NEGY) {

            unsigned short& actualLocalID = blocksLocalIDs_[firstIndex][-1][secondIndex];
            oldLocalID = actualLocalID;
            unsigned short oldLocalID = actualLocalID;
            unsigned int oldGlobalID = actualLocalID ? palette_.getT2(actualLocalID) : 0;
            const block& oldB = block::getBlockC(oldGlobalID);
            placeNewBlock(actualLocalID, b);
            blockWasModified = oldLocalID != actualLocalID;
            needsRemesh_ = needsRemesh_ || blockWasModified && blocksLocalIDs_[firstIndex][0][secondIndex];
            if (!oldLocalID && actualLocalID)
                nTotalBlocksMinusY_++;
            else if (oldLocalID && !actualLocalID)
                nTotalBlocksMinusY_--;
            nOpaqueBlocksMinusY_ += (b.opacity() == blockOpacity::OPAQUEBLOCK) - (oldB.opacity() == blockOpacity::OPAQUEBLOCK);

        }
        else if (neighbor == blockViewDir::PLUSZ) {

            unsigned short& actualLocalID = blocksLocalIDs_[firstIndex][secondIndex][CHUNK_SIZE];
            oldLocalID = actualLocalID;
            unsigned short oldLocalID = actualLocalID;
            unsigned int oldGlobalID = actualLocalID ? palette_.getT2(actualLocalID) : 0;
            const block& oldB = block::getBlockC(oldGlobalID);
            placeNewBlock(actualLocalID, b);
            blockWasModified = oldLocalID != actualLocalID;
            needsRemesh_ = needsRemesh_ || blockWasModified && blocksLocalIDs_[firstIndex][secondIndex][CHUNK_SIZE_LIMIT];
            if (!oldLocalID && actualLocalID)
                nTotalBlocksPlusZ_++;
            else if (oldLocalID && !actualLocalID)
                nTotalBlocksPlusZ_--;
            nOpaqueBlocksPlusZ_ += (b.opacity() == blockOpacity::OPAQUEBLOCK) - (oldB.opacity() == blockOpacity::OPAQUEBLOCK);

        }
        else if (neighbor == blockViewDir::NEGZ) {

            unsigned short& actualLocalID = blocksLocalIDs_[firstIndex][secondIndex][-1];
            oldLocalID = actualLocalID;
            unsigned short oldLocalID = actualLocalID;
            unsigned int oldGlobalID = actualLocalID ? palette_.getT2(actualLocalID) : 0;
            const block& oldB = block::getBlockC(oldGlobalID);
            placeNewBlock(actualLocalID, b);
            blockWasModified = oldLocalID != actualLocalID;
            needsRemesh_ = needsRemesh_ || blockWasModified && blocksLocalIDs_[firstIndex][secondIndex][0];
            if (!oldLocalID && actualLocalID)
                nTotalBlocksMinusZ_++;
            else if (oldLocalID && !actualLocalID)
                nTotalBlocksMinusZ_--;
            nOpaqueBlocksMinusZ_ += (b.opacity() == blockOpacity::OPAQUEBLOCK) - (oldB.opacity() == blockOpacity::OPAQUEBLOCK);

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

    bool chunk::renewMesh(bool generationRemesh) {

        std::unique_lock<std::shared_mutex> lock(renderingDataMutex_);

        if (needsRemesh_) {

            needsRemesh_ = false;

            model* chunkModel = nullptr;

            renderingData_.vertices = model();
            renderingData_.translucentVertices = model();
            renderingData_.pointLights = std::vector<lightInstance>();
            renderingData_.spotLights = std::vector<lightInstance>();

            // Read chunk data section starts.
            blocksMutex_.lock_shared();

            // COSAS QUE HACER.
            // 1º. SOPORTE PARA LOS BORDES DE LOS CHUNKS.
            // AFTER VIDEO. METER BLOCK VOLUMETRIC LIGHTING, SUN VOLUMETRIC LIGHTING & CONE LIGHTS (ESTO ULTIMO NECESITARA METER EL TIPO DE LUZ EN FLOODLIGHTPROPINSTANCE)

            int x = 0,
                y = 0,
                z = 0;
            unsigned short localID = 0;

            // Render faces that do not require data from neighbor chunks.
            bool blockHasLight = false;
            bool xIs0 = false;
            bool yIs0 = false;
            bool zIs0 = false;
            bool xIsChunkLimit = false;
            bool yIsChunkLimit = false;
            bool zIsChunkLimit = false;
            vertex aux;
            const block* bNeighbor = nullptr;
            unsigned short neighborLocalID = 0;
            if (nTotalBlocks_ && nOpaqueBlocks_ < nBlocksChunk)
                for (x = 0; x < CHUNK_SIZE; x++)
                    for (y = 0; y < CHUNK_SIZE; y++)
                        for (z = 0; z < CHUNK_SIZE; z++) {

                            xIs0 = x == 0;
                            yIs0 = y == 0;
                            zIs0 = z == 0;
                            xIsChunkLimit = x == CHUNK_SIZE_LIMIT;
                            yIsChunkLimit = y == CHUNK_SIZE_LIMIT;
                            zIsChunkLimit = z == CHUNK_SIZE_LIMIT;

                            localID = blocksLocalIDs_[x][y][z];
                            block& b = localID ? block::getBlockC(palette_.getT2(localID)) : block::emptyBlock();
                            const varRef& emittedLight = b.emittedLight();

                            blockHasLight = blockLightLevel_[x][y][z] > 0;

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
                                            aux.lightExtraData.z = bNeighbor->getMaterialIndex();

                                            switch (vertex)
                                            {
                                                case 0: // block vertex 0 (B)
                                                    aux.additionalData = (blockLightColor_[x][y][z] + (xIs0 ? basicVec4Zeroes : blockLightColor_[x - 1][y][z]) + (yIs0 ? blockLightColor_[x][y - 1][z] : basicVec4Zeroes) + ((xIs0 || yIs0) ? basicVec4Zeroes : blockLightColor_[x - 1][y - 1][z])) / 4;
                                                    aux.lightExtraData.x = 0;
                                                    aux.lightExtraData.y = 0;
                                                    break;
                                                case 1: // block vertex 3 (C)
                                                case 4:
                                                    aux.additionalData = (blockLightColor_[x][y][z] + (xIs0 ? basicVec4Zeroes : blockLightColor_[x - 1][y][z]) + (yIsChunkLimit ? basicVec4Zeroes : blockLightColor_[x][y + 1][z]) + ((xIs0 || yIsChunkLimit) ? basicVec4Zeroes : blockLightColor_[x - 1][y + 1][z])) / 4;
                                                    aux.lightExtraData.x = 0;
                                                    aux.lightExtraData.y = 1;
                                                    break;
                                                case 2: // block vertex 1 (A)
                                                case 3:
                                                    aux.additionalData = (blockLightColor_[x][y][z] + (xIsChunkLimit ? basicVec4Zeroes : blockLightColor_[x + 1][y][z]) + (yIs0 ? basicVec4Zeroes : blockLightColor_[x][y - 1][z]) + ((xIsChunkLimit || yIs0) ? basicVec4Zeroes : blockLightColor_[x + 1][y - 1][z])) / 4;
                                                    aux.lightExtraData.x = 1;
                                                    aux.lightExtraData.y = 0;
                                                    break;
                                                case 5: // block vertex 2 (D)
                                                    aux.additionalData = (blockLightColor_[x][y][z] + (xIsChunkLimit ? basicVec4Zeroes : blockLightColor_[x + 1][y][z]) + (yIsChunkLimit ? basicVec4Zeroes : blockLightColor_[x][y + 1][z]) + ((xIsChunkLimit || yIsChunkLimit) ? basicVec4Zeroes : blockLightColor_[x + 1][y + 1][z])) / 4;
                                                    aux.lightExtraData.x = 0;
                                                    aux.lightExtraData.y = 0;
                                                    break;
                                            }

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
                                            aux.lightExtraData.z = bNeighbor->getMaterialIndex();

                                            switch (vertex)
                                            {
                                                case 0: // block vertex 5 (B)
                                                    aux.additionalData = (blockLightColor_[x][y][z] + (xIsChunkLimit ? basicVec4Zeroes : blockLightColor_[x+1][y][z]) + (yIs0 ? basicVec4Zeroes : blockLightColor_[x][y-1][z]) + ((xIsChunkLimit || yIs0) ? basicVec4Zeroes : blockLightColor_[x+1][y-1][z])) / 4;
                                                    aux.lightExtraData.x = 0;
                                                    aux.lightExtraData.y = 0;
                                                    break;
                                                case 1: // block vertex 6 (C)
                                                case 4:
                                                    aux.additionalData = (blockLightColor_[x][y][z] + (xIsChunkLimit ? basicVec4Zeroes : blockLightColor_[x+1][y][z]) + (yIsChunkLimit ? basicVec4Zeroes : blockLightColor_[x][y+1][z]) + ((xIsChunkLimit || yIsChunkLimit) ? basicVec4Zeroes : blockLightColor_[x+1][y+1][z])) / 4;
                                                    aux.lightExtraData.x = 0;
                                                    aux.lightExtraData.y = 1;
                                                    break;
                                                case 2: // block vertex 4 (A)
                                                case 3:
                                                    aux.additionalData = (blockLightColor_[x][y][z] + (xIs0 ? basicVec4Zeroes : blockLightColor_[x-1][y][z]) + (yIs0 ? basicVec4Zeroes : blockLightColor_[x][y-1][z]) + ((xIs0 || yIs0) ? basicVec4Zeroes : blockLightColor_[x-1][y-1][z])) / 4;
                                                    aux.lightExtraData.x = 1;
                                                    aux.lightExtraData.y = 0;
                                                    break;
                                                case 5: // block vertex 7 (D)
                                                    aux.additionalData = (blockLightColor_[x][y][z] + (xIs0 ? basicVec4Zeroes : blockLightColor_[x-1][y][z]) + (yIsChunkLimit ? basicVec4Zeroes : blockLightColor_[x][y+1][z]) + ((xIs0 || yIsChunkLimit) ? basicVec4Zeroes : blockLightColor_[x-1][y+1][z])) / 4;
                                                    aux.lightExtraData.x = 0;
                                                    aux.lightExtraData.y = 0;
                                                    break;
                                            }

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
                                            aux.lightExtraData.z = bNeighbor->getMaterialIndex();
                                            
                                            switch (vertex)
                                            {
                                                case 0: // block vertex 1 (B)
                                                    aux.additionalData = (blockLightColor_[x][y][z] + (xIsChunkLimit ? basicVec4Zeroes : blockLightColor_[x + 1][y][z]) + (zIs0 ? basicVec4Zeroes : blockLightColor_[x][y][z - 1]) + ((xIsChunkLimit || zIs0) ? basicVec4Zeroes : blockLightColor_[x + 1][y][z - 1])) / 4;
                                                    aux.lightExtraData.x = 0;
                                                    aux.lightExtraData.y = 0;
                                                    break;
                                                case 1: // block vertex 5 (C)
                                                case 4:
                                                    aux.additionalData = (blockLightColor_[x][y][z] + (xIsChunkLimit ? basicVec4Zeroes : blockLightColor_[x+1][y][z]) + (zIsChunkLimit ? basicVec4Zeroes : blockLightColor_[x][y][z+1]) + ((xIsChunkLimit || zIsChunkLimit) ? basicVec4Zeroes : blockLightColor_[x+1][y][z+1])) / 4;
                                                    aux.lightExtraData.x = 0;
                                                    aux.lightExtraData.y = 1;
                                                    break;
                                                case 2: // block vertex 0 (A)
                                                case 3:
                                                    aux.additionalData = (blockLightColor_[x][y][z] + (xIs0 ? basicVec4Zeroes : blockLightColor_[x-1][y][z]) + (zIs0 ? basicVec4Zeroes : blockLightColor_[x][y][z-1]) + ((xIs0 || zIs0) ? basicVec4Zeroes : blockLightColor_[x-1][y][z-1])) / 4;
                                                    aux.lightExtraData.x = 1;
                                                    aux.lightExtraData.y = 0;
                                                    break;
                                                case 5: // block vertex 4 (D)
                                                    aux.additionalData = (blockLightColor_[x][y][z] + (xIs0 ? basicVec4Zeroes : blockLightColor_[x-1][y][z]) + (zIsChunkLimit ? basicVec4Zeroes : blockLightColor_[x][y][z+1]) + ((xIs0 || zIsChunkLimit) ? basicVec4Zeroes : blockLightColor_[x-1][y][z+1])) / 4;
                                                    aux.lightExtraData.x = 0;
                                                    aux.lightExtraData.y = 0;
                                                    break;
                                            }

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
                                            aux.lightExtraData.z = bNeighbor->getMaterialIndex();

                                            switch (vertex) 
                                            {
                                                case 0: // block vertex 3 (B)
                                                    aux.additionalData = (blockLightColor_[x][y][z] + (xIs0 ? basicVec4Zeroes : blockLightColor_[x-1][y][z]) + (zIs0 ? basicVec4Zeroes : blockLightColor_[x][y][z-1]) + ((xIs0 || zIs0) ? basicVec4Zeroes : blockLightColor_[x-1][y][z-1])) / 4;
                                                    aux.lightExtraData.x = 0;
                                                    aux.lightExtraData.y = 0;
                                                    break;
                                                case 1: // block vertex 7 (C)
                                                case 4:
                                                    aux.additionalData = (blockLightColor_[x][y][z] + (xIs0 ? basicVec4Zeroes : blockLightColor_[x-1][y][z]) + (zIsChunkLimit ? basicVec4Zeroes : blockLightColor_[x][y][z+1]) + ((xIs0 || zIsChunkLimit) ? basicVec4Zeroes : blockLightColor_[x-1][y][z+1])) / 4;
                                                    aux.lightExtraData.x = 0;
                                                    aux.lightExtraData.y = 1;
                                                    break;
                                                case 2: // block vertex 2 (A)
                                                case 3:
                                                    aux.additionalData = (blockLightColor_[x][y][z] + (xIsChunkLimit ? basicVec4Zeroes : blockLightColor_[x+1][y][z]) + (zIs0 ? basicVec4Zeroes : blockLightColor_[x][y][z-1]) + ((xIsChunkLimit || zIs0) ? basicVec4Zeroes : blockLightColor_[x+1][y][z-1])) / 4;
                                                    aux.lightExtraData.x = 1;
                                                    aux.lightExtraData.y = 0;
                                                    break;
                                                case 5: // block vertex 6 (D)
                                                    aux.additionalData = (blockLightColor_[x][y][z] + (xIsChunkLimit ? basicVec4Zeroes : blockLightColor_[x+1][y][z]) + (zIsChunkLimit ? basicVec4Zeroes : blockLightColor_[x][y][z+1]) + ((xIsChunkLimit || zIsChunkLimit) ? basicVec4Zeroes : blockLightColor_[x+1][y][z+1])) / 4;
                                                    aux.lightExtraData.x = 0;
                                                    aux.lightExtraData.y = 0;
                                                    break;
                                            }

                                            chunkModel->push_back(aux);

                                        }

                                        // Add texture to the face.
                                        models::addBlockFaceTexture(*bNeighbor, *chunkModel, "faceY+");

                                    }

                                }

                                // Draw face for block at x + 1.
                                if (x < CHUNK_SIZE_LIMIT && (neighborLocalID = blocksLocalIDs_[x + 1][y][z])) {

                                    bNeighbor = &block::getBlockC(palette_.getT2(neighborLocalID));

                                    if (b != *bNeighbor) {

                                        chunkModel = (bNeighbor->opacity() == blockOpacity::TRANSLUCENTBLOCK) ? &renderingData_.translucentVertices : &renderingData_.vertices;

                                        // Create the face's vertices for face x-.
                                        for (int vertex = 0; vertex < blockTriangles_->operator[](0).size(); vertex++) {

                                            aux.positions[0] = chunkPos_.x * CHUNK_SIZE + x + 1 + blockVertices_->operator[](blockTriangles_->operator[](4)[vertex]).positions[0];
                                            aux.positions[1] = chunkPos_.y * CHUNK_SIZE + y + blockVertices_->operator[](blockTriangles_->operator[](4)[vertex]).positions[1];
                                            aux.positions[2] = chunkPos_.z * CHUNK_SIZE + z + blockVertices_->operator[](blockTriangles_->operator[](4)[vertex]).positions[2];
                                            aux.lightExtraData.z = bNeighbor->getMaterialIndex();

                                            switch (vertex)
                                            {
                                                case 0: // block vertex 4 (B)
                                                    aux.additionalData = (blockLightColor_[x][y][z] + (zIsChunkLimit ? basicVec4Zeroes : blockLightColor_[x][y][z+1]) + (yIs0 ? basicVec4Zeroes : blockLightColor_[x][y-1][z]) + ((yIs0 || zIsChunkLimit) ? basicVec4Zeroes : blockLightColor_[x][y-1][z+1])) / 4;
                                                    aux.lightExtraData.x = 0;
                                                    aux.lightExtraData.y = 0;
                                                    break;
                                                case 1: // block vertex 7 (C)
                                                case 4: 
                                                    aux.additionalData = (blockLightColor_[x][y][z] + (zIsChunkLimit ? basicVec4Zeroes : blockLightColor_[x][y][z+1]) + (yIsChunkLimit ? basicVec4Zeroes : blockLightColor_[x][y+1][z]) + ((yIsChunkLimit || zIsChunkLimit) ? basicVec4Zeroes : blockLightColor_[x][y+1][z+1])) / 4;
                                                    aux.lightExtraData.x = 0;
                                                    aux.lightExtraData.y = 1;
                                                    break;
                                                case 2: // block vertex 0 (A)
                                                case 3: 
                                                    aux.additionalData = (blockLightColor_[x][y][z] + (zIs0 ? basicVec4Zeroes : blockLightColor_[x][y][z-1]) + (yIs0 ? basicVec4Zeroes : blockLightColor_[x][y-1][z]) + ((yIs0 || zIs0) ? basicVec4Zeroes : blockLightColor_[x][y-1][z-1])) / 4;
                                                    aux.lightExtraData.x = 1;
                                                    aux.lightExtraData.y = 0;
                                                    break;
                                                case 5: // block vertex 3 (D)
                                                    aux.additionalData = (blockLightColor_[x][y][z] + (zIs0 ? basicVec4Zeroes : blockLightColor_[x][y][z-1]) + (yIsChunkLimit ? basicVec4Zeroes : blockLightColor_[x][y+1][z]) + ((yIsChunkLimit || zIs0) ? basicVec4Zeroes : blockLightColor_[x][y+1][z-1])) / 4;
                                                    aux.lightExtraData.x = 0;
                                                    aux.lightExtraData.y = 0;
                                                    break;
                                            }

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
                                            aux.lightExtraData.z = bNeighbor->getMaterialIndex();

                                            switch (vertex)
                                            {
                                                case 0: // block vertex 1 (B)
                                                    aux.additionalData = (blockLightColor_[x][y][z] + (zIs0 ? basicVec4Zeroes : blockLightColor_[x][y][z-1]) + (yIs0 ? basicVec4Zeroes : blockLightColor_[x][y-1][z]) + ((yIs0 || zIs0) ? basicVec4Zeroes : blockLightColor_[x][y-1][z-1])) / 4;
                                                    aux.lightExtraData.x = 0;
                                                    aux.lightExtraData.y = 0;
                                                    break;
                                                case 1: // block vertex 2 (C)
                                                case 4:
                                                    aux.additionalData = (blockLightColor_[x][y][z] + (zIs0 ? basicVec4Zeroes : blockLightColor_[x][y][z-1]) + (yIsChunkLimit ? basicVec4Zeroes : blockLightColor_[x][y+1][z]) + ((yIsChunkLimit || zIs0) ? basicVec4Zeroes : blockLightColor_[x][y+1][z-1])) / 4;
                                                    aux.lightExtraData.x = 0;
                                                    aux.lightExtraData.y = 1;
                                                    break;
                                                case 2: // block vertex 5 (A)
                                                case 3: 
                                                    aux.additionalData = (blockLightColor_[x][y][z] + (zIsChunkLimit ? basicVec4Zeroes : blockLightColor_[x][y][z+1]) + (yIs0 ? basicVec4Zeroes : blockLightColor_[x][y-1][z]) + ((yIs0 || zIsChunkLimit) ? basicVec4Zeroes : blockLightColor_[x][y-1][z+1])) / 4;
                                                    aux.lightExtraData.x = 1;
                                                    aux.lightExtraData.y = 0;
                                                    break;
                                                case 5: // block vertex 6 (D)
                                                    aux.additionalData = (blockLightColor_[x][y][z] + (zIsChunkLimit ? basicVec4Zeroes : blockLightColor_[x][y][z+1]) + (yIsChunkLimit ? basicVec4Zeroes : blockLightColor_[x][y+1][z]) + ((yIsChunkLimit || zIsChunkLimit) ? basicVec4Zeroes : blockLightColor_[x][y+1][z+1])) / 4;
                                                    aux.lightExtraData.x = 0;
                                                    aux.lightExtraData.y = 0;
                                                    break;
                                            }

                                            chunkModel->push_back(aux);

                                        }

                                        // Add texture to the face.
                                        models::addBlockFaceTexture(*bNeighbor, *chunkModel, "faceX+");

                                    }

                                }

                            }

                        }

            if (nTotalBlocksPlusZ_) {
            
                for (x = 0; x < CHUNK_SIZE; x++)
                    for (y = 0; y < CHUNK_SIZE; y++) {

                        localID = blocksLocalIDs_[x][y][CHUNK_SIZE_LIMIT];
                        const block& b = localID ? block::getBlockC(palette_.getT2(localID)) : block::emptyBlock();

                        if (b.opacity() <= blockOpacity::TRANSLUCENTBLOCK && (neighborLocalID = blocksLocalIDs_[x][y][CHUNK_SIZE])) {

                            // Front face vertices with culling of non-visible faces. z-
                            bNeighbor = &block::getBlockC(palette_.getT2(neighborLocalID));

                            if (b != *bNeighbor) {

                                chunkModel = (bNeighbor->opacity() == blockOpacity::TRANSLUCENTBLOCK) ? &renderingData_.translucentVertices : &renderingData_.vertices;

                                // Create the face's vertices for face z-.
                                for (int vertex = 0; vertex < blockTriangles_->operator[](0).size(); vertex++) {

                                    aux.positions[0] = chunkPos_.x * CHUNK_SIZE + x + blockVertices_->operator[](blockTriangles_->operator[](0)[vertex]).positions[0];
                                    aux.positions[1] = chunkPos_.y * CHUNK_SIZE + y + blockVertices_->operator[](blockTriangles_->operator[](0)[vertex]).positions[1];
                                    aux.positions[2] = (chunkPos_.z + 1) * CHUNK_SIZE + blockVertices_->operator[](blockTriangles_->operator[](0)[vertex]).positions[2];
                                    aux.lightExtraData.x = bNeighbor->getMaterialIndex();

                                    chunkModel->push_back(aux);

                                }

                                // Add texture to the face.
                                models::addBlockFaceTexture(*bNeighbor, *chunkModel, "faceZ-");

                            }

                        }

                    }
            
            }

            if (nTotalBlocksMinusZ_) {

                for (x = 0; x < CHUNK_SIZE; x++)
                    for (y = 0; y < CHUNK_SIZE; y++) {

                        localID = blocksLocalIDs_[x][y][0];
                        const block& b = localID ? block::getBlockC(palette_.getT2(localID)) : block::emptyBlock();

                        if (b.opacity() <= blockOpacity::TRANSLUCENTBLOCK && (neighborLocalID = blocksLocalIDs_[x][y][-1])) {

                            bNeighbor = &block::getBlockC(palette_.getT2(neighborLocalID));

                            if (b != *bNeighbor) {

                                chunkModel = (bNeighbor->opacity() == blockOpacity::TRANSLUCENTBLOCK) ? &renderingData_.translucentVertices : &renderingData_.vertices;

                                // Create the face's vertices for z+.
                                for (int vertex = 0; vertex < blockTriangles_->operator[](0).size(); vertex++) {

                                    aux.positions[0] = chunkPos_.x * CHUNK_SIZE + x + blockVertices_->operator[](blockTriangles_->operator[](1)[vertex]).positions[0];
                                    aux.positions[1] = chunkPos_.y * CHUNK_SIZE + y + blockVertices_->operator[](blockTriangles_->operator[](1)[vertex]).positions[1];
                                    aux.positions[2] = (chunkPos_.z - 1) * CHUNK_SIZE + (16 - 1) + blockVertices_->operator[](blockTriangles_->operator[](1)[vertex]).positions[2];
                                    aux.lightExtraData.x = bNeighbor->getMaterialIndex();

                                    chunkModel->push_back(aux);

                                }

                                // Add texture to the face.
                                models::addBlockFaceTexture(*bNeighbor, *chunkModel, "faceZ+");

                            }

                        }

                    }

            }

            if (nTotalBlocksPlusY_) {

                for (x = 0; x < CHUNK_SIZE; x++)
                    for (z = 0; z < CHUNK_SIZE; z++) {

                        localID = blocksLocalIDs_[x][CHUNK_SIZE_LIMIT][z];
                        const block& b = localID ? block::getBlockC(palette_.getT2(localID)) : block::emptyBlock();

                        if (b.opacity() <= blockOpacity::TRANSLUCENTBLOCK && (neighborLocalID = blocksLocalIDs_[x][CHUNK_SIZE][z])) {

                            bNeighbor = &block::getBlockC(palette_.getT2(neighborLocalID));

                            if (b != *bNeighbor) {

                                chunkModel = (bNeighbor->opacity() == blockOpacity::TRANSLUCENTBLOCK) ? &renderingData_.translucentVertices : &renderingData_.vertices;

                                // Create the face's vertices for face y-.
                                for (int vertex = 0; vertex < blockTriangles_->operator[](0).size(); vertex++) {

                                    aux.positions[0] = chunkPos_.x * CHUNK_SIZE + x + blockVertices_->operator[](blockTriangles_->operator[](3)[vertex]).positions[0];
                                    aux.positions[1] = (chunkPos_.y + 1) * CHUNK_SIZE + blockVertices_->operator[](blockTriangles_->operator[](3)[vertex]).positions[1];
                                    aux.positions[2] = chunkPos_.z * CHUNK_SIZE + z + blockVertices_->operator[](blockTriangles_->operator[](3)[vertex]).positions[2];
                                    aux.lightExtraData.x = bNeighbor->getMaterialIndex();

                                    chunkModel->push_back(aux);

                                }

                                // Add texture to the face.
                                models::addBlockFaceTexture(*bNeighbor, *chunkModel, "faceY-");

                            }

                        }

                    }

            }
            
            if (nTotalBlocksMinusY_) {

                for (x = 0; x < CHUNK_SIZE; x++)
                    for (z = 0; z < CHUNK_SIZE; z++) {

                        localID = blocksLocalIDs_[x][0][z];
                        const block& b = localID ? block::getBlockC(palette_.getT2(localID)) : block::emptyBlock();

                        if (b.opacity() <= blockOpacity::TRANSLUCENTBLOCK && (neighborLocalID = blocksLocalIDs_[x][-1][z])) {

                            bNeighbor = &block::getBlockC(palette_.getT2(neighborLocalID));

                            if (b != *bNeighbor) {

                                chunkModel = (bNeighbor->opacity() == blockOpacity::TRANSLUCENTBLOCK) ? &renderingData_.translucentVertices : &renderingData_.vertices;

                                // Create the face's vertices for face y+.
                                for (int vertex = 0; vertex < blockTriangles_->operator[](0).size(); vertex++) {

                                    aux.positions[0] = chunkPos_.x * CHUNK_SIZE + x + blockVertices_->operator[](blockTriangles_->operator[](2)[vertex]).positions[0];
                                    aux.positions[1] = (chunkPos_.y - 1) * CHUNK_SIZE + (16 - 1) + blockVertices_->operator[](blockTriangles_->operator[](2)[vertex]).positions[1];
                                    aux.positions[2] = chunkPos_.z * CHUNK_SIZE + z + blockVertices_->operator[](blockTriangles_->operator[](2)[vertex]).positions[2];
                                    aux.lightExtraData.x = bNeighbor->getMaterialIndex();

                                    chunkModel->push_back(aux);

                                }

                                // Add texture to the face.
                                models::addBlockFaceTexture(*bNeighbor, *chunkModel, "faceY+");

                            }

                        }

                    }

            }

            if (nTotalBlocksPlusX_) {

                for (y = 0; y < CHUNK_SIZE; y++)
                    for (z = 0; z < CHUNK_SIZE; z++) {

                        localID = blocksLocalIDs_[CHUNK_SIZE_LIMIT][y][z];
                        const block& b = localID ? block::getBlockC(palette_.getT2(localID)) : block::emptyBlock();

                        if (b.opacity() <= blockOpacity::TRANSLUCENTBLOCK && (neighborLocalID = blocksLocalIDs_[CHUNK_SIZE][y][z])) {

                            bNeighbor = &block::getBlockC(palette_.getT2(neighborLocalID));

                            if (b != *bNeighbor) {

                                chunkModel = (bNeighbor->opacity() == blockOpacity::TRANSLUCENTBLOCK) ? &renderingData_.translucentVertices : &renderingData_.vertices;

                                // Create the face's vertices for face x-.
                                for (int vertex = 0; vertex < blockTriangles_->operator[](0).size(); vertex++) {

                                    aux.positions[0] = (chunkPos_.x + 1) * CHUNK_SIZE + blockVertices_->operator[](blockTriangles_->operator[](4)[vertex]).positions[0];
                                    aux.positions[1] = chunkPos_.y * CHUNK_SIZE + y + blockVertices_->operator[](blockTriangles_->operator[](4)[vertex]).positions[1];
                                    aux.positions[2] = chunkPos_.z * CHUNK_SIZE + z + blockVertices_->operator[](blockTriangles_->operator[](4)[vertex]).positions[2];
                                    aux.lightExtraData.x = bNeighbor->getMaterialIndex();

                                    chunkModel->push_back(aux);

                                }

                                // Add texture to the face.
                                models::addBlockFaceTexture(*bNeighbor, *chunkModel, "faceX-");

                            }

                        }

                    }

            }

            if (nTotalBlocksMinusX_) {

                for (y = 0; y < CHUNK_SIZE; y++)
                    for (z = 0; z < CHUNK_SIZE; z++) {

                        localID = blocksLocalIDs_[0][y][z];
                        const block& b = localID ? block::getBlockC(palette_.getT2(localID)) : block::emptyBlock();

                        if (b.opacity() <= blockOpacity::TRANSLUCENTBLOCK && (neighborLocalID = blocksLocalIDs_[-1][y][z]))  {

                            bNeighbor = &block::getBlockC(palette_.getT2(neighborLocalID));

                            if (b != *bNeighbor) {

                                chunkModel = (bNeighbor->opacity() == blockOpacity::TRANSLUCENTBLOCK) ? &renderingData_.translucentVertices : &renderingData_.vertices;

                                // Create the face's vertices for face x+.
                                for (int vertex = 0; vertex < blockTriangles_->operator[](0).size(); vertex++) {

                                    aux.positions[0] = (chunkPos_.x - 1) * CHUNK_SIZE + (16 - 1) + blockVertices_->operator[](blockTriangles_->operator[](5)[vertex]).positions[0];
                                    aux.positions[1] = chunkPos_.y * CHUNK_SIZE + y + blockVertices_->operator[](blockTriangles_->operator[](5)[vertex]).positions[1];
                                    aux.positions[2] = chunkPos_.z * CHUNK_SIZE + z + blockVertices_->operator[](blockTriangles_->operator[](5)[vertex]).positions[2];
                                    aux.lightExtraData.x = bNeighbor->getMaterialIndex();

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

        status(chunkStatus::MESHED);

        renderingData_.totalSize = renderingData_.vertices.size() + renderingData_.translucentVertices.size();

        return renderingData_.totalSize;

    }

    void chunk::clearBlockLight() {
    
        std::memset(blockLightColor_, 0, nBlocksChunk * sizeof(basicVec4));
        std::memset(blockLightLevel_, -1, nBlocksChunk * sizeof(char));
    
    }

    void chunk::recalculateBlockLight() {

        // Get neighbor info of all the chunk's neighbors.
        std::unordered_map<vec3, neighborsInfo*> cacheNeighborsInfo;
        chunkManager::chunkNeighborsInfoMutex().lock();
        for (const vec3& offset : neighborsOffsets) 
            cacheNeighborsInfo[offset] = &chunkManager::chunkNeighborsInfo()[chunkPos_ + offset];
        chunkManager::chunkNeighborsInfoMutex().unlock();

        // Update block light render information from blocks within the chunk.
        int x = 0,
            y = 0,
            z = 0;
        unsigned short localID = 0;
        vec3 neighborOffset;
        std::deque<blockLightMod> floodLightsInstances;
        bool blockLightChecked[CHUNK_SIZE][CHUNK_SIZE][CHUNK_SIZE];
        neighborsInfo* neighborsInfoPtr = nullptr;
        threadsafe<std::list<blockLightMod>>* list = nullptr;
        blockLightMod* mod = nullptr;

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

                    const pointLight& light = *(emittedLight.pointer<pointLight>());

                    // Search for blocks affected by this light.
                    std::memset(blockLightChecked, 0, nBlocksChunk * sizeof(bool));
                    floodLightsInstances.clear();
                    floodLightsInstances.emplace_back(basicVec3{ static_cast<char>(x), static_cast<char>(y), static_cast<char>(z) },
                        light.maxDistance(), light.color());

                    while (floodLightsInstances.size() > 0) {

                        // Get next light and reset some variables.
                        blockLightMod& floodLight = floodLightsInstances.front();
                        basicVec3& pos = floodLight.pos;
                        neighborOffset.x = pos.x >= CHUNK_SIZE_LIMIT ? 1 : pos.x <= 0 ? -1 : 0;
                        neighborOffset.y = pos.y >= CHUNK_SIZE_LIMIT ? 1 : pos.y <= 0 ? -1 : 0;
                        neighborOffset.z = pos.z >= CHUNK_SIZE_LIMIT ? 1 : pos.z <= 0 ? -1 : 0;

                        if (floodLight.intensity > 0 && !blockLightChecked[pos.x][pos.y][pos.z]) {

                            float lightLevelScale = floodLight.intensity / 8.0f; // 8 is the maximum allowed light level.
                            blockLightColor_[pos.x][pos.y][pos.z] += floodLight.color * lightLevelScale;
                            blockLightLevel_[pos.x][pos.y][pos.z] = floodLight.intensity;

                            blockLightChecked[pos.x][pos.y][pos.z] = true;

                            //+x
                            if (pos.x < CHUNK_SIZE_LIMIT && blocksLocalIDs_[pos.x + 1][pos.y][pos.z] == 0) {

                                floodLightsInstances.emplace_back(basicVec3{ pos.x + 1, pos.y, pos.z }, floodLight.intensity - 1, floodLight.color);

                            }
                            else if (neighborOffset.x == 1) {
                            
                                passLightToNeighbor(cacheNeighborsInfo, floodLight, pos, {1, 0, 0});

                                if (neighborOffset.y == 1) {
                                
                                    passLightToNeighbor(cacheNeighborsInfo, floodLight, pos, { 1, 1, 0 });

                                    if (neighborOffset.z == 1) {
                                    
                                        passLightToNeighbor(cacheNeighborsInfo, floodLight, pos, { 1, 1, 1 });
                                    
                                    }
                                    else if (neighborOffset.z == -1) {
                                    
                                        passLightToNeighbor(cacheNeighborsInfo, floodLight, pos, { 1, 1, -1 });
                                    
                                    }
                                
                                }
                                else if (neighborOffset.y == -1) {
                                
                                    passLightToNeighbor(cacheNeighborsInfo, floodLight, pos, { 1, -1, 0 });

                                    if (neighborOffset.z == 1) {

                                        passLightToNeighbor(cacheNeighborsInfo, floodLight, pos, { 1, -1, 1 });

                                    }
                                    else if (neighborOffset.z == -1) {

                                        passLightToNeighbor(cacheNeighborsInfo, floodLight, pos, { 1, -1, -1 });

                                    }
                                
                                }
                                else {
                                
                                    if (neighborOffset.z == 1) {

                                        passLightToNeighbor(cacheNeighborsInfo, floodLight, pos, { 1, 0, 1 });

                                    }
                                    else if (neighborOffset.z == -1) {

                                        passLightToNeighbor(cacheNeighborsInfo, floodLight, pos, { 1, 0, -1 });

                                    }
                                
                                }

                            }

                            //-x
                            if (pos.x > 0 && blocksLocalIDs_[pos.x - 1][pos.y][pos.z] == 0) {

                                floodLightsInstances.emplace_back(basicVec3{ pos.x - 1, pos.y, pos.z }, floodLight.intensity - 1, floodLight.color);

                            }
                            else if (neighborOffset.x == -1) {

                                passLightToNeighbor(cacheNeighborsInfo, floodLight, pos, { -1, 0, 0 });

                                if (neighborOffset.y == 1) {
                                
                                    passLightToNeighbor(cacheNeighborsInfo, floodLight, pos, { -1, 1, 0 });

                                    if (neighborOffset.z == 1) {
                                    
                                        passLightToNeighbor(cacheNeighborsInfo, floodLight, pos, { -1, 1, 1 });
                                    
                                    }
                                    else if (neighborOffset.z == -1) {
                                    
                                        passLightToNeighbor(cacheNeighborsInfo, floodLight, pos, { -1, 1, -1 });
                                    
                                    }
                                
                                }
                                else if (neighborOffset.y == -1) {
                                
                                    passLightToNeighbor(cacheNeighborsInfo, floodLight, pos, { -1, -1, 0 });

                                    if (neighborOffset.z == 1) {

                                        passLightToNeighbor(cacheNeighborsInfo, floodLight, pos, { -1, -1, 1 });

                                    }
                                    else if (neighborOffset.z == -1) {

                                        passLightToNeighbor(cacheNeighborsInfo, floodLight, pos, { -1, -1, -1 });

                                    }
                                
                                }
                                else {
                                
                                    if (neighborOffset.z == 1) {

                                        passLightToNeighbor(cacheNeighborsInfo, floodLight, pos, { -1, 0, 1 });

                                    }
                                    else if (neighborOffset.z == -1) {

                                        passLightToNeighbor(cacheNeighborsInfo, floodLight, pos, { -1, 0, -1 });

                                    }
                                
                                }

                            }

                            //+y
                            if (pos.y < CHUNK_SIZE_LIMIT && blocksLocalIDs_[pos.x][pos.y + 1][pos.z] == 0) {

                                floodLightsInstances.emplace_back(basicVec3{ pos.x, pos.y + 1, pos.z }, floodLight.intensity - 1, floodLight.color);

                            }
                            else if (neighborOffset.y == 1) {

                                passLightToNeighbor(cacheNeighborsInfo, floodLight, pos, { 0, 1, 0 });

                                if (neighborOffset.z == 1) {

                                    passLightToNeighbor(cacheNeighborsInfo, floodLight, pos, { 0, 1, 1 });

                                }
                                else if (neighborOffset.z == -1) {

                                    passLightToNeighbor(cacheNeighborsInfo, floodLight, pos, { 0, 1, -1 });

                                }

                            }

                            //-y
                            if (pos.y > 0 && blocksLocalIDs_[pos.x][pos.y - 1][pos.z] == 0) {

                                floodLightsInstances.emplace_back(basicVec3{ pos.x, pos.y - 1, pos.z }, floodLight.intensity - 1, floodLight.color);

                            }
                            else if (neighborOffset.y == -1) {

                                passLightToNeighbor(cacheNeighborsInfo, floodLight, pos, { 0, -1, 0 });

                                if (neighborOffset.z == 1) {

                                    passLightToNeighbor(cacheNeighborsInfo, floodLight, pos, { 0, -1, 1 });

                                }
                                else if (neighborOffset.z == -1) {

                                    passLightToNeighbor(cacheNeighborsInfo, floodLight, pos, { 0, -1, -1 });

                                }

                            }

                            //+z
                            if (pos.z < CHUNK_SIZE_LIMIT && blocksLocalIDs_[pos.x][pos.y][pos.z + 1] == 0) {

                                floodLightsInstances.emplace_back(basicVec3{ pos.x, pos.y, pos.z + 1 }, floodLight.intensity - 1, floodLight.color);

                            }
                            else if (neighborOffset.z == 1) {

                                passLightToNeighbor(cacheNeighborsInfo, floodLight, pos, { 0, 0, 1 });

                            }

                            //-z
                            if (pos.z > 0 && blocksLocalIDs_[pos.x][pos.y][pos.z - 1] == 0) {

                                floodLightsInstances.emplace_back(basicVec3{ pos.x, pos.y, pos.z - 1 }, floodLight.intensity - 1, floodLight.color);

                            }
                            else if (neighborOffset.z == -1) {

                                passLightToNeighbor(cacheNeighborsInfo, floodLight, pos, { 0, 0, -1 });

                            }

                        }

                        floodLightsInstances.pop_front();

                    }

                }

            }

        }
    }

    void chunk::recalculateNeighborBlockLight() {

        // Update block light render information from blocks within the chunk.
        unsigned short localID = 0;
        vec3 neighborOffset;
        std::deque<blockLightMod> floodLightsInstances;

        bool blockLightChecked[CHUNK_SIZE][CHUNK_SIZE][CHUNK_SIZE];

        chunkManager::chunkNeighborsInfoMutex().lock();
        neighborsInfo& neighborsInfoPtr = chunkManager::chunkNeighborsInfo()[chunkPos_];
        chunkManager::chunkNeighborsInfoMutex().unlock();

        neighborsInfoPtr.blockLightsFromNeighbor.lock();
        blockLightsByNeighbor& blockLightsByNeighborPtr = neighborsInfoPtr.blockLightsFromNeighbor.get();
        neighborsInfoPtr.blockLightsFromNeighbor.unlock();
        for (auto it = blockLightsByNeighborPtr.begin(); it != blockLightsByNeighborPtr.end(); it++) {

            it->second.lock();
            const std::list<blockLightMod>& blockLightMods = it->second.get();

            for (auto itLight = blockLightMods.cbegin(); itLight != blockLightMods.cend(); itLight++) {
          
                const basicVec3& blockLightPos = itLight->pos;

                localID = blocksLocalIDs_[blockLightPos.x][blockLightPos.y][blockLightPos.z];
                block& b = localID ? block::getBlockC(palette_.getT2(localID)) : block::emptyBlock();

                // Add block's light.
                if (b.opacity() < blockOpacity::OPAQUEBLOCK) {

                    // Search for blocks affected by this light.
                    std::memset(blockLightChecked, 0, nBlocksChunk * sizeof(bool));
                    floodLightsInstances.clear();
                    floodLightsInstances.push_back(*itLight);

                    while (floodLightsInstances.size() > 0) {

                        // Get next light and reset some variables.
                        blockLightMod& floodLight = floodLightsInstances.front();
                        basicVec3& pos = floodLight.pos;
                        neighborOffset.x = pos.x >= CHUNK_SIZE_LIMIT ? 1 : pos.x <= 0 ? -1 : 0;
                        neighborOffset.y = pos.y >= CHUNK_SIZE_LIMIT ? 1 : pos.y <= 0 ? -1 : 0;
                        neighborOffset.z = pos.z >= CHUNK_SIZE_LIMIT ? 1 : pos.z <= 0 ? -1 : 0;

                        if (floodLight.intensity > 0 && !blockLightChecked[pos.x][pos.y][pos.z]) {

                            float lightLevelScale = floodLight.intensity / 8.0f; // 8 is the maximum allowed light level.
                            blockLightColor_[pos.x][pos.y][pos.z] += floodLight.color * lightLevelScale;
                            blockLightLevel_[pos.x][pos.y][pos.z] = floodLight.intensity;

                            blockLightChecked[pos.x][pos.y][pos.z] = true;

                            //+x
                            if (pos.x < CHUNK_SIZE_LIMIT && blocksLocalIDs_[pos.x + 1][pos.y][pos.z] == 0) {

                                floodLightsInstances.emplace_back(basicVec3{ pos.x + 1, pos.y, pos.z }, floodLight.intensity - 1, floodLight.color);

                            }
 
                            //-x
                            if (pos.x > 0 && blocksLocalIDs_[pos.x - 1][pos.y][pos.z] == 0) {

                                floodLightsInstances.emplace_back(basicVec3{ pos.x - 1, pos.y, pos.z }, floodLight.intensity - 1, floodLight.color);

                            }

                            //+y
                            if (pos.y < CHUNK_SIZE_LIMIT && blocksLocalIDs_[pos.x][pos.y + 1][pos.z] == 0) {

                                floodLightsInstances.emplace_back(basicVec3{ pos.x, pos.y + 1, pos.z }, floodLight.intensity - 1, floodLight.color);

                            }

                            //-y
                            if (pos.y > 0 && blocksLocalIDs_[pos.x][pos.y - 1][pos.z] == 0) {

                                floodLightsInstances.emplace_back(basicVec3{ pos.x, pos.y - 1, pos.z }, floodLight.intensity - 1, floodLight.color);

                            }

                            //+z
                            if (pos.z < CHUNK_SIZE_LIMIT && blocksLocalIDs_[pos.x][pos.y][pos.z + 1] == 0) {

                                floodLightsInstances.emplace_back(basicVec3{ pos.x, pos.y, pos.z + 1 }, floodLight.intensity - 1, floodLight.color);

                            }

                            //-z
                            if (pos.z > 0 && blocksLocalIDs_[pos.x][pos.y][pos.z - 1] == 0) {

                                floodLightsInstances.emplace_back(basicVec3{ pos.x, pos.y, pos.z - 1 }, floodLight.intensity - 1, floodLight.color);

                            }

                        }

                        floodLightsInstances.pop_front();

                    }

                }
            
            }

            it->second.unlock();

        }
        
    }

    void chunk::makeEmpty() {
    
        blocksMutex_.lock();

        needsRemesh_ = true;
        nOpaqueBlocks_ = 0;
        nOpaqueBlocksPlusX_ = 0;
        nOpaqueBlocksMinusX_ = 0;
        nOpaqueBlocksPlusY_ = 0;
        nOpaqueBlocksMinusY_ = 0;
        nOpaqueBlocksPlusZ_ = 0;
        nOpaqueBlocksMinusZ_ = 0;
        nTotalBlocks_ = 0;
        nTotalBlocksPlusX_ = 0;
        nTotalBlocksMinusX_ = 0;
        nTotalBlocksPlusY_ = 0;
        nTotalBlocksMinusY_ = 0;
        nTotalBlocksPlusZ_ = 0;
        nTotalBlocksMinusZ_ = 0;

        blocksLocalIDs_.fill(0);

        floodPointLightPositions_.clear();

        palette_.clear();
        paletteCount_.clear();

        blocksMutex_.unlock();

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

    void chunk::placeNewBlock(unsigned short& actualLocalID, const block& newBlock) {

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

    void chunk::passLightToNeighbor(std::unordered_map<vec3, neighborsInfo*>& cacheNeighborsInfo, blockLightMod& floodLight, basicVec3& pos,
        const vec3& neighborOffset) {
    
        neighborsInfo* neighborsInfoPtr = cacheNeighborsInfo[neighborOffset]; // Get the neighbor.
        neighborsInfoPtr->blockLightsFromNeighbor.lock();
        threadsafe<std::list<blockLightMod>>* list =
            &neighborsInfoPtr->blockLightsFromNeighbor.get()[basicVec3{ (char)-neighborOffset.x, (char)-neighborOffset.y, (char)-neighborOffset.z }]; // And pass it the data from this chunk.
        neighborsInfoPtr->blockLightsFromNeighbor.unlock();

        list->lock();
        blockLightMod* mod = &list->get().emplace_back();
        mod->color = floodLight.color;
        mod->intensity = floodLight.intensity - 1;
        mod->pos.x = neighborOffset.x == 1 ? 0 : neighborOffset.x == -1 ? CHUNK_SIZE_LIMIT : pos.x;
        mod->pos.y = neighborOffset.y == 1 ? 0 : neighborOffset.y == -1 ? CHUNK_SIZE_LIMIT : pos.y;
        mod->pos.z = neighborOffset.z == 1 ? 0 : neighborOffset.z == -1 ? CHUNK_SIZE_LIMIT : pos.z;
        list->unlock();
    
    }

    // 'chunkEvent' class.

    void chunkEvent::notify(const vec2& chunkPosXZ) {

        chunkPosXZ_ = chunkPosXZ;

        event::notify();

    }


    // 'chunkManager' class.

    bool chunkManager::initialised_ = false;
    bool chunkManager::chunkJobSystemOverloaded_ = false;
    int chunkManager::nChunksToCompute_ = 0;

    std::atomic<bool> chunkManager::clearChunksFlag_ = false;
    std::atomic<bool> chunkManager::priorityUpdatesRemaining_ = false;

    std::atomic<bool> chunkManager::waitInitialTerrainLoaded_ = true;

    std::unordered_map<vec3, chunk*> chunkManager::clientChunks_;

    std::unordered_map<vec3, chunk*> chunkManager::simulatedChunks_;

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

    atomicRecyclingPool<job>* chunkManager::loadChunkJobs_;
    atomicRecyclingPool<chunk> chunkManager::chunksPool_;

    chunkEvent chunkManager::onChunkLoad_("On chunk load");
    chunkEvent chunkManager::onChunkUnload_("On chunk unload");

    std::mutex chunkManager::chunkNeighborsInfoMutex_;
    std::unordered_map<vec3, neighborsInfo> chunkManager::chunkNeighborsInfo_;

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

            chunkTasks_ = new threadPool(game::getSettings().maxChunkhreads());
            priorityChunkTasks_ = new threadPool(2);
            loadChunkJobs_ = new atomicRecyclingPool<job>(game::getSettings().maxChunkhreads());
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

        return (clientChunks_.find(chunkPos) != clientChunks_.cend()) ? clientChunks_.at(chunkPos)->status() : chunkStatus::NOTLOADED;

    }

    bool chunkManager::isEmptyBlock(int posX, int posY, int posZ) {

        const block* selectedBlock;
        std::unique_lock<std::recursive_mutex> lock(chunksMutex_);
        
        selectedBlock = &getBlockOGWorld_(posX, posY, posZ);

        return selectedBlock->isEmptyBlock();

    }

    bool chunkManager::chunkInRenderDistance(const vec3& chunkPos) {

        vec3 distancePlayer = chunkDistance(chunkPos, playerChunkPosCopy_);
        return distancePlayer.x <= nChunksToCompute_ &&
               distancePlayer.z <= nChunksToCompute_ &&
               distancePlayer.y <= yChunksRange;

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
        if (mode == engineMode::INITLEVEL || mode == engineMode::EDITLEVEL)
            nChunksToCompute_ = nChunksToCompute;
        else
            logger::errorLog("Cannot change the number of chunks to compute in the current engine mode " + std::to_string((unsigned int)mode));
    
    }

    const block& chunkManager::setBlock(int x, int y, int z, const block& blockID) {

        vec3 chunkPos = getChunkCoords(x, y, z);
        std::unique_lock<std::recursive_mutex> lock(chunksMutex_);
        
        if (clientChunks_.find(chunkPos) == clientChunks_.cend())
            logger::errorLog("Chunk " + std::to_string(chunkPos.x) + "|" + std::to_string(chunkPos.y) + "|" + std::to_string(chunkPos.z) + " does not exist");
        else {
            
            block removedBlock = clientChunks_.at(chunkPos)->setBlock(getChunkRelCoords(x, y, z), blockID);

            return removedBlock;
            
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

            issueChunkMeshJob(chunkJobType::UNLOADANDSAVE, unloadedChunk, true);

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

    void chunkManager::remesh(chunk* c, bool isPriorityUpdate, bool remeshPostGeneration) {
    
        unsigned int meshSize = c->renewMesh(remeshPostGeneration);
        pushNewChunkMesh(isPriorityUpdate, c, meshSize);

    }

    void chunkManager::renewMesh(const vec3& chunkPos, bool isPriorityUpdate, bool remeshPostGeneration) {
    
        chunksMutex_.lock();

        auto it = clientChunks_.find(chunkPos);
        if (it == clientChunks_.cend()) {

            chunksMutex_.unlock();

            logger::errorLog("The chunk " + std::to_string(chunkPos.x) + "|" + std::to_string(chunkPos.y) + "|"
                + std::to_string(chunkPos.z) + " is not registered");
        
        }
        else {
            
            chunksMutex_.unlock();

            remesh(it->second, isPriorityUpdate, remeshPostGeneration);

        }
            
    }

    void chunkManager::manageChunks() {

        {
            // NEXT. LOS JOBS DE NEIGHBOR QUE SE TIENE QUE ACTUALIZAR CON PRIORITY DEBEN IR EN UN MISMO JOB SINO DA PROBLEMAS.

            std::unique_lock<std::mutex> lock(managerThreadMutex_),
                priorityUpdatesLock(priorityUpdatesRemainingMutex_);

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
                        else {

                            chunk* c = getChunk(chunkPos);

                            if (ensureChunkIfVisible(chunkPos + blockViewDir::PLUSX) +
                                ensureChunkIfVisible(chunkPos + blockViewDir::NEGX) +
                                ensureChunkIfVisible(chunkPos + blockViewDir::PLUSY) +
                                ensureChunkIfVisible(chunkPos + blockViewDir::NEGY) +
                                ensureChunkIfVisible(chunkPos + blockViewDir::PLUSZ) +
                                ensureChunkIfVisible(chunkPos + blockViewDir::NEGZ) == 6) {

                                continueCreatingChunks = ++nIterations < maxIterations;

                                frontierChunksSet_.erase(*frontierIt_);
                                frontierIt_ = frontierChunks_.erase(frontierIt_);

                            }
                            else
                                frontierIt_++;
                        }
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

    void chunkManager::openedTerrainFileName(const std::string& newFilename) {
          
        if (game::selectedEngineMode() == VoxelEng::engineMode::EDITLEVEL)
            logger::errorLog("Cannot change the opened terrain file name while in a level");
        else
            openedTerrainFileName_ = newFilename;

    }

    bool chunkManager::ensureChunkIfVisible(const vec3& chunkPos) {
       
        bool chunkInDistance = chunkInRenderDistance(chunkPos);

        chunksMutex_.lock();
        bool cExists = clientChunks_.contains(chunkPos);
        chunksMutex_.unlock();

        if (chunkInDistance)
            return cExists ? true : loadChunk(chunkPos) != nullptr;
        else
            return false;
        
    }

    std::string chunkManager::serializeChunk(chunk* c) {

        timer t;
        t.start();

        c->blockDataMutex().lock_shared();

        // Save local ID data (including duplicated data about neighbors).
        const Padded3DArray<unsigned short>& blocks = c->blocks();
        std::string data(c->blocks().size() * sizeof(unsigned short), 0);
        std::memcpy(data.data(), c->blocks().data(), c->blocks().size() * sizeof(unsigned short));

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

        data += std::to_string(c->nOpaqueBlocks()) + '|' + std::to_string(c->nOpaqueBlocksPlusX()) + '|' + std::to_string(c->nOpaqueBlocksMinusX()) + '|' + std::to_string(c->nOpaqueBlocksPlusY()) + '|' + std::to_string(c->nOpaqueBlocksMinusY()) + '|' + std::to_string(c->nOpaqueBlocksPlusZ()) + '|' + std::to_string(c->nOpaqueBlocksMinusZ()) + '|';
        
        data += '@';
        
        data += std::to_string(c->nTotalBlocks()) + '|' + std::to_string(c->nTotalBlocksPlusX()) + '|' + std::to_string(c->nTotalBlocksMinusX()) + '|' + std::to_string(c->nTotalBlocksPlusY()) + '|' + std::to_string(c->nTotalBlocksMinusY()) + '|' + std::to_string(c->nTotalBlocksPlusZ()) + '|' + std::to_string(c->nTotalBlocksMinusZ()) + '|';

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

        Padded3DArray<unsigned short>& blocks = chunk->blocks();

        std::memcpy(blocks.data(), dataBegin, blocks.size() * sizeof(unsigned short));
        index += blocks.size() * sizeof(unsigned short) + 1; // +1 to skip the '@' delimiter character.

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
        while (c != '@') {

            while (c != '|') {

                word += c;

                c = data[++index];

            }

            switch (state) {

            case 0:
                chunk->nOpaqueBlocks(sto<unsigned short>(word));
                break;
            case 1:
                chunk->nOpaqueBlocksPlusX(sto<unsigned short>(word));
                break;
            case 2:
                chunk->nOpaqueBlocksMinusX(sto<unsigned short>(word));
                break;
            case 3:
                chunk->nOpaqueBlocksPlusY(sto<unsigned short>(word));
                break;
            case 4:
                chunk->nOpaqueBlocksMinusY(sto<unsigned short>(word));
                break;
            case 5:
                chunk->nOpaqueBlocksPlusZ(sto<unsigned short>(word));
                break;
            case 6:
                chunk->nOpaqueBlocksMinusZ(sto<unsigned short>(word));
                break;

            }

            word.clear();
            c = data[++index];
            state++;

        }

        c = data[++index]; // Skip the '@' delimiter character.
        state = 0;
        while (index < nBytes) {

            while (c != '|') {

                word += c;

                c = data[++index];

            }

            switch (state) {

            case 0:
                chunk->nTotalBlocks(sto<unsigned short>(word));
                break;
            case 1:
                chunk->nTotalBlocksPlusX(sto<unsigned short>(word));
                break;
            case 2:
                chunk->nTotalBlocksMinusX(sto<unsigned short>(word));
                break;
            case 3:
                chunk->nTotalBlocksPlusY(sto<unsigned short>(word));
                break;
            case 4:
                chunk->nTotalBlocksMinusY(sto<unsigned short>(word));
                break;
            case 5:
                chunk->nTotalBlocksPlusZ(sto<unsigned short>(word));
                break;
            case 6:
                chunk->nTotalBlocksMinusZ(sto<unsigned short>(word));
                break;

            }

            word.clear();
            c = data[++index];
            state++;

        }

        chunk->loadedFromDisk(true);

        chunk->clearBlockLight();

        chunk->recalculateBlockLight();

        chunk->needsRemesh(true); // TODO. EL BUG ES QUE SI MODIFICO UN BLOCK EN UN BORDE, TAMBIEN HAY QUE GUARDAR EL CHUNK VECINO QUE LE HACE FRONTERA.
    
    }

    chunk* chunkManager::loadChunk(const vec3& chunkPos) {

        chunksMutex_.lock();
        auto cIt = clientChunks_.find(chunkPos);
        chunksMutex_.unlock();

        if (cIt == clientChunks_.cend()) {
        
            chunk* c = &chunksPool_.get();

            // TODO. DO A CHUNK.RESET() NON-STATIC METHOD.
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
            issueChunkMeshJob(chunkJobType::LOAD, c, true);

            return c;
        
        }
        else
            return cIt->second;

    }

    void chunkManager::issueChunkMeshJob(chunkJobType type, void* data, bool pushJobBack) {
    
        job* aJob = &loadChunkJobs_->get();

        // Set the task.
        switch (type) {
        
            case chunkJobType::NONE:
                logger::errorLog("No chunk job type was specified");
                break;
            case chunkJobType::LOAD:
                aJob->setTask(loadChunkJob, data, loadChunkJobs_);
                break;
            case chunkJobType::LOAD2:
                aJob->setTask(loadChunkJobPass2, data, loadChunkJobs_);
                break;
            case chunkJobType::ONLYREMESH:
                aJob->setTask(remeshChunkJob, data, loadChunkJobs_);
                break;
            case chunkJobType::UNLOADANDSAVE:
                aJob->setTask(unloadAndSaveChunkJob, data, loadChunkJobs_);
                break;
            case chunkJobType::PRIORITYREMESH:
                aJob->setTask(priorityRemeshChunkJob, data, loadChunkJobs_);
                break;
            default:
                logger::errorLog("Unsupported chunkJobType type " + std::to_string(static_cast<int>(type)));
                break;

        }

        // Send the job to its corresponding queue.

        switch (type) {

            case chunkJobType::NONE:
                logger::errorLog("No chunk job type was specified");
                break;
            
            case chunkJobType::PRIORITYREMESH:
                priorityChunkTasks_->submitJob(aJob, pushJobBack);
                break;
            case chunkJobType::LOAD:
            case chunkJobType::LOAD2:
            case chunkJobType::ONLYREMESH:
            case chunkJobType::UNLOADANDSAVE:
                chunkTasks_->submitJob(aJob, pushJobBack);
                break;
            default:
                logger::errorLog("Unsupported chunkJobType type " + std::to_string(static_cast<int>(type)));
                break;

        }
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

        chunkNeighborsInfo_.clear();

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
            return clientChunks_.at(chunkPos)->getBlock(floorMod(posX, CHUNK_SIZE), floorMod(posY, CHUNK_SIZE), floorMod(posZ, CHUNK_SIZE), true);
    
    }

    void chunkManager::pushNewChunkMesh(bool isPriorityUpdate, chunk* c, std::size_t meshSize) {
    
        if (isPriorityUpdate) {

            priorityNewChunkMeshesMutex_.lock();
            priorityNewChunkMeshes_.push_back(c);
            priorityNewChunkMeshesMutex_.unlock();

        }
        else if (meshSize) {

            newChunkMeshesMutex_.lock();
            newChunkMeshes_.push_back(c);
            newChunkMeshesMutex_.unlock();

        }
    
    }

    void chunkManager::loadChunkJob(void* data) {

        chunk* c = static_cast<chunk*>(data);

        c->makeEmpty();
        if (world::isSaved(c->chunkPos())) {  // Load previously saved chunk. Skips remaining loading passes.
        
            deserializeChunk(c, world::loadChunk(c->chunkPos()));

        }   
        else { // Generate new chunk.
        
            worldGen::generate(*c);
            
        }

        c->status(chunkStatus::BASICTERRAIN);

        onLoadChunkJobFinish(c);

    }

    void chunkManager::onLoadChunkJobFinish(chunk* c) {
    
        const vec3& chunkPos = c->chunkPos();

        chunkNeighborsInfoMutex_.lock();
        neighborsInfo& info = chunkNeighborsInfo_[chunkPos]; // TODO. LOS NEIGHBORINFO DEBEN DESAPARECER CUANDO UN CHUNK SE DESCARGA.
        chunkNeighborsInfoMutex_.unlock();

        if (++info.neighborsGenPass1Completed_ >= 27)
            issueChunkMeshJob(chunkJobType::LOAD2, c);

        vec3 neighborPos;
        for (const vec3& offset : neighborsOffsets) {
        
            neighborPos = chunkPos + offset;

            chunkNeighborsInfoMutex_.lock();
            neighborsInfo& infoNeighbor = chunkNeighborsInfo_[neighborPos];
            chunkNeighborsInfoMutex_.unlock();

            chunksMutex_.lock();
            chunk* neighbor = clientChunks_.contains(neighborPos) ? clientChunks_[neighborPos] : nullptr;
            chunksMutex_.unlock();

            if (++infoNeighbor.neighborsGenPass1Completed_ >= 27 && neighbor)
                issueChunkMeshJob(chunkJobType::LOAD2, neighbor);
        
        }

    }

    void chunkManager::loadChunkJobPass2(void* data) {

        chunk* c = static_cast<chunk*>(data);

        worldGen::genPass2(*c);

        c->status(chunkStatus::DECORATED);

        remesh(c, false, true);

    }

    void chunkManager::remeshChunkJob(void* data) {

        chunk* c = static_cast<chunk*>(data);

        remesh(c, false, false);

    }

    void chunkManager::unloadAndSaveChunkJob(void* data) {

        chunk* c = static_cast<chunk*>(data);

        if (c->modified()) {

            world::saveChunk(c);
            c->modified(false);

        }

        c->status(chunkStatus::NOTLOADED);
            
        chunksPool_.free(*c);

    }

    void chunkManager::priorityRemeshChunkJob(void* data) {

        chunk* c = static_cast<chunk*>(data);

        c->needsRemesh(true);

        remesh(c, true, false);

        priorityNewChunkMeshesCV_.notify_all();

    }

}
