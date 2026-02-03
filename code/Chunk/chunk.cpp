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
#include <tuple>

#include <camera.h>
#include <player.h>
#include <input.h>
#include <gui.h>
#include <game.h>
#include <Chunk/chunkNeighbors.hpp>
#include <Time/Timer/timer.h>
#include <Graphics/graphics.h>
#include <Graphics/Lighting/Lights/light.hpp>
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
        isOpaque_(16, 16, 16, 1, false),
        blockLightColor_(16, 16, 16, 1, basicVec4Zero),
        blockLightLevel_(16, 16, 16, 1, 0),
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
        unloadWhenNoOwners_(false),
        loadStatus_(chunkLoadStatus::NOTLOADED),
        chunkPos_(vec3Zero) {}

    chunk::chunk(bool empty, const ivec3& chunkPos)
        : blocksLocalIDs_(16, 16, 16, 1, 0),
        isOpaque_(16, 16, 16, 1, false),
        blockLightColor_(16, 16, 16, 1, basicVec4Zero),
        blockLightLevel_(16, 16, 16, 1, 0),
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
        unloadWhenNoOwners_(false),
        loadStatus_(chunkLoadStatus::NOTLOADED),
        chunkPos_(vec3Zero) {

        if (!empty)
            worldGen::generate(*this); // This can only call to setBlock to modify the chunk and that method already takes care of 'blocksMutex_'.

    }

    chunk::chunk(chunk& c)
        : blocksLocalIDs_(c.blocksLocalIDs_),
        isOpaque_(c.isOpaque_),
        blockLightColor_(c.blockLightColor_),
        blockLightLevel_(c.blockLightLevel_),
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
        owners_(c.owners_),
        unloadWhenNoOwners_(c.unloadWhenNoOwners_),
        loadStatus_(c.loadStatus_),
        needsRemesh_(c.needsRemesh_.load()),
        loadedFromDisk_(c.loadedFromDisk_.load()),
        chunkPos_(c.chunkPos_),
        ownedChunks_(c.ownedChunks_) {

        c.blocksMutex_.lock_shared();

        renderingData_ = c.renderingData_;

        palette_ = c.palette_;
        paletteCount_ = c.paletteCount_;
        freeLocalIDs_ = c.freeLocalIDs_;

        renderingData_.vertices = c.renderingData_.vertices;

        c.blocksMutex_.unlock_shared();

    }

    template <>
    const blockState& chunk::get<blockProperty>(GLbyte x, GLbyte y, GLbyte z) {

        unsigned int localID = blocksLocalIDs_[x][y][z];
        return blockState(localID ? block::getBlockC(palette_.getT2(localID)) : block::emptyBlock(), blockProperty::emptyProp());

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

    const block& chunk::setBlock(sbyte x, sbyte y, sbyte z, const block& b, bool modification) {

        bool isNewBlockOpaque = b.opacity() == blockOpacity::OPAQUEBLOCK;
        unsigned short& actualLocalID = blocksLocalIDs_[x][y][z];
        unsigned short oldLocalID = actualLocalID;
        unsigned int oldGlobalID = actualLocalID ? palette_.getT2(actualLocalID) : 0;
        const block& oldB = block::getBlockC(oldGlobalID);
        ivec3 neighborOffset;

        const bool blockWasModified = placeNewBlock_(actualLocalID, b);

        needsRemesh_ = needsRemesh_ || blockWasModified;
        modified_ = modified_ || (blockWasModified && modification);

        isOpaque_[x][y][z] = isNewBlockOpaque;

        neighborOffset.x = x >= CHUNK_SIZE ? 1 : x <= -1 ? -1 : 0;
        neighborOffset.y = y >= CHUNK_SIZE ? 1 : y <= -1 ? -1 : 0;
        neighborOffset.z = z >= CHUNK_SIZE ? 1 : z <= -1 ? -1 : 0;
        if (neighborOffset == ivec3Zero) {

            if (!oldLocalID && actualLocalID)
                nTotalBlocks_++;
            else if (oldLocalID && !actualLocalID)
                nTotalBlocks_--;
            nOpaqueBlocks_ += (isNewBlockOpaque)-(oldB.opacity() == blockOpacity::OPAQUEBLOCK);

            replaceBlockLight(oldB, b, x, y, z);

        }
        else if (neighborOffset == ivec3FixedNorth) {

            if (!oldLocalID && actualLocalID)
                nTotalBlocksPlusX_++;
            else if (oldLocalID && !actualLocalID)
                nTotalBlocksPlusX_--;

            nOpaqueBlocksPlusX_ += (isNewBlockOpaque)-(oldB.opacity() == blockOpacity::OPAQUEBLOCK);

        }
        else if (neighborOffset == ivec3FixedSouth) {

            if (!oldLocalID && actualLocalID)
                nTotalBlocksMinusX_++;
            else if (oldLocalID && !actualLocalID)
                nTotalBlocksMinusX_--;
            nOpaqueBlocksMinusX_ += (isNewBlockOpaque)-(oldB.opacity() == blockOpacity::OPAQUEBLOCK);

        }
        else if (neighborOffset == ivec3FixedUp) {

            if (!oldLocalID && actualLocalID)
                nTotalBlocksPlusY_++;
            else if (oldLocalID && !actualLocalID)
                nTotalBlocksPlusY_--;
            nOpaqueBlocksPlusY_ += (isNewBlockOpaque)-(oldB.opacity() == blockOpacity::OPAQUEBLOCK);

        }
        else if (neighborOffset == ivec3FixedDown) {

            if (!oldLocalID && actualLocalID)
                nTotalBlocksMinusY_++;
            else if (oldLocalID && !actualLocalID)
                nTotalBlocksMinusY_--;
            nOpaqueBlocksMinusY_ += (isNewBlockOpaque)-(oldB.opacity() == blockOpacity::OPAQUEBLOCK);

        }
        else if (neighborOffset == ivec3FixedEast) {

            if (!oldLocalID && actualLocalID)
                nTotalBlocksPlusZ_++;
            else if (oldLocalID && !actualLocalID)
                nTotalBlocksPlusZ_--;
            nOpaqueBlocksPlusZ_ += (isNewBlockOpaque)-(oldB.opacity() == blockOpacity::OPAQUEBLOCK);

        }
        else if (neighborOffset == ivec3FixedWest) {

            if (!oldLocalID && actualLocalID)
                nTotalBlocksMinusZ_++;
            else if (oldLocalID && !actualLocalID)
                nTotalBlocksMinusZ_--;
            nOpaqueBlocksMinusZ_ += (isNewBlockOpaque)-(oldB.opacity() == blockOpacity::OPAQUEBLOCK);

        }

        return oldB;

    }

    void chunk::replaceBlockLight(const block& oldB, const block& b, const ivec3& pos) {

        // Remove old light pos if any.
        if (!oldB.emittedLight().isNull())
            floodPointLightPositions_.erase(pos);

        setBlockLight(b, pos);

    }

    void chunk::setBlockLight(const block& b, const ivec3& pos) {

        const varRef& emittedLight = b.emittedLight();
        if (!emittedLight.isNull()) { // Add new light pos if any.

            floodPointLightPositions_.insert(pos);

            const pointLight* light = emittedLight.pointer<pointLight>();
            blockLightColor_[pos.x][pos.y][pos.z] = light->ambient();
            blockLightLevel_[pos.x][pos.y][pos.z] = light->maxDistance();

        }

    }

    void chunk::chunkPos(const ivec3& newChunkPos) {

        chunkPos_ = newChunkPos;
        renderingData_.extraRenderingData.globalChunkPos.x = newChunkPos.x * CHUNK_SIZE + CHUNK_SIZE / 2;
        renderingData_.extraRenderingData.globalChunkPos.y = newChunkPos.y * CHUNK_SIZE + CHUNK_SIZE / 2;
        renderingData_.extraRenderingData.globalChunkPos.z = newChunkPos.z * CHUNK_SIZE + CHUNK_SIZE / 2;

    }

    std::size_t chunk::renewMesh(bool forceRemesh) {

        std::unique_lock<std::shared_mutex> lock(renderingDataMutex_);

        if (forceRemesh || needsRemesh_) {

            needsRemesh_ = false;

            model* chunkModel = nullptr;

            renderingData_.vertices = model();
            renderingData_.translucentVertices = model();

            // Read chunk data section starts.
            blocksMutex_.lock_shared();

            // Render faces that do not require data from neighbor chunks.
            bool blockHasLight = false;
            int x = 0,
                y = 0,
                z = 0;
            unsigned short localID = 0;
            unsigned short neighborLocalID = 0;
            vertex aux;
            const block* bNeighbor = nullptr;
            if (nTotalBlocks_ && nOpaqueBlocks_ < nBlocksChunk)
                for (x = 0; x < CHUNK_SIZE; x++)
                    for (y = 0; y < CHUNK_SIZE; y++)
                        for (z = 0; z < CHUNK_SIZE; z++) {

                            localID = blocksLocalIDs_[x][y][z];
                            block& b = localID ? block::getBlockC(palette_.getT2(localID)) : block::emptyBlock();
                            const varRef& emittedLight = b.emittedLight();

                            blockHasLight = blockLightLevel_[x][y][z] > 0;

                            // Add block's model to the mesh if necessary.
                            if (b.opacity() <= blockOpacity::TRANSLUCENTBLOCK) {

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
                                                aux.additionalData = getBlockLightAverage_(blockLightColor_[x][y][z],
                                                    basicVec3(x, y, z + 1), basicVec3(x, y - 1, z), basicVec3(x, y - 1, z + 1));
                                                aux.lightExtraData.x = 0;
                                                aux.lightExtraData.y = 0;
                                                break;
                                            case 1: // block vertex 7 (C)
                                            case 4:
                                                aux.additionalData = getBlockLightAverage_(blockLightColor_[x][y][z],
                                                    basicVec3(x, y, z + 1), basicVec3(x, y + 1, z), basicVec3(x, y + 1, z + 1));
                                                aux.lightExtraData.x = 0;
                                                aux.lightExtraData.y = 1;
                                                break;
                                            case 2: // block vertex 0 (A)
                                            case 3:
                                                aux.additionalData = getBlockLightAverage_(blockLightColor_[x][y][z],
                                                    basicVec3(x, y, z - 1), basicVec3(x, y - 1, z), basicVec3(x, y - 1, z - 1));
                                                aux.lightExtraData.x = 1;
                                                aux.lightExtraData.y = 0;
                                                break;
                                            case 5: // block vertex 3 (D)
                                                aux.additionalData = getBlockLightAverage_(blockLightColor_[x][y][z],
                                                    basicVec3(x, y, z - 1), basicVec3(x, y + 1, z), basicVec3(x, y + 1, z - 1));
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
                                                aux.additionalData = getBlockLightAverage_(blockLightColor_[x][y][z],
                                                    basicVec3(x, y, z - 1), basicVec3(x, y - 1, z), basicVec3(x, y - 1, z - 1));
                                                aux.lightExtraData.x = 0;
                                                aux.lightExtraData.y = 0;
                                                break;
                                            case 1: // block vertex 2 (C)
                                            case 4:
                                                aux.additionalData = getBlockLightAverage_(blockLightColor_[x][y][z],
                                                    basicVec3(x, y, z - 1), basicVec3(x, y + 1, z), basicVec3(x, y + 1, z - 1));
                                                aux.lightExtraData.x = 0;
                                                aux.lightExtraData.y = 1;
                                                break;
                                            case 2: // block vertex 5 (A)
                                            case 3:
                                                aux.additionalData = getBlockLightAverage_(blockLightColor_[x][y][z],
                                                    basicVec3(x, y, z + 1), basicVec3(x, y - 1, z), basicVec3(x, y - 1, z + 1));
                                                aux.lightExtraData.x = 1;
                                                aux.lightExtraData.y = 0;
                                                break;
                                            case 5: // block vertex 6 (D)
                                                aux.additionalData = getBlockLightAverage_(blockLightColor_[x][y][z],
                                                    basicVec3(x, y, z + 1), basicVec3(x, y + 1, z), basicVec3(x, y + 1, z + 1));
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
                                                aux.additionalData = getBlockLightAverage_(blockLightColor_[x][y][z],
                                                    basicVec3(x + 1, y, z), basicVec3(x, y, z - 1), basicVec3(x + 1, y, z - 1));
                                                aux.lightExtraData.x = 0;
                                                aux.lightExtraData.y = 0;
                                                break;
                                            case 1: // block vertex 5 (C)
                                            case 4:
                                                aux.additionalData = getBlockLightAverage_(blockLightColor_[x][y][z],
                                                    basicVec3(x + 1, y, z), basicVec3(x, y, z + 1), basicVec3(x + 1, y, z + 1));
                                                aux.lightExtraData.x = 0;
                                                aux.lightExtraData.y = 1;
                                                break;
                                            case 2: // block vertex 0 (A)
                                            case 3:
                                                aux.additionalData = getBlockLightAverage_(blockLightColor_[x][y][z],
                                                    basicVec3(x - 1, y, z), basicVec3(x, y, z - 1), basicVec3(x - 1, y, z - 1));
                                                aux.lightExtraData.x = 1;
                                                aux.lightExtraData.y = 0;
                                                break;
                                            case 5: // block vertex 4 (D)
                                                aux.additionalData = getBlockLightAverage_(blockLightColor_[x][y][z],
                                                    basicVec3(x - 1, y, z), basicVec3(x, y, z + 1), basicVec3(x - 1, y, z + 1));
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
                                                aux.additionalData = getBlockLightAverage_(blockLightColor_[x][y][z],
                                                    basicVec3(x - 1, y, z), basicVec3(x, y, z - 1), basicVec3(x - 1, y, z - 1));
                                                aux.lightExtraData.x = 0;
                                                aux.lightExtraData.y = 0;
                                                break;
                                            case 1: // block vertex 7 (C)
                                            case 4:
                                                aux.additionalData = getBlockLightAverage_(blockLightColor_[x][y][z],
                                                    basicVec3(x - 1, y, z), basicVec3(x, y, z + 1), basicVec3(x - 1, y, z + 1));
                                                aux.lightExtraData.x = 0;
                                                aux.lightExtraData.y = 1;
                                                break;
                                            case 2: // block vertex 2 (A)
                                            case 3:
                                                aux.additionalData = getBlockLightAverage_(blockLightColor_[x][y][z],
                                                    basicVec3(x + 1, y, z), basicVec3(x, y, z - 1), basicVec3(x + 1, y, z - 1));
                                                aux.lightExtraData.x = 1;
                                                aux.lightExtraData.y = 0;
                                                break;
                                            case 5: // block vertex 6 (D)
                                                aux.additionalData = getBlockLightAverage_(blockLightColor_[x][y][z],
                                                    basicVec3(x + 1, y, z), basicVec3(x, y, z + 1), basicVec3(x + 1, y, z + 1));
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
                                                aux.additionalData = getBlockLightAverage_(blockLightColor_[x][y][z],
                                                    basicVec3(x - 1, y, z), basicVec3(x, y - 1, z), basicVec3(x - 1, y - 1, z));
                                                aux.lightExtraData.x = 0;
                                                aux.lightExtraData.y = 0;
                                                break;
                                            case 1: // block vertex 3 (C)
                                            case 4:
                                                aux.additionalData = getBlockLightAverage_(blockLightColor_[x][y][z],
                                                    basicVec3(x - 1, y, z), basicVec3(x, y + 1, z), basicVec3(x - 1, y + 1, z));
                                                aux.lightExtraData.x = 0;
                                                aux.lightExtraData.y = 1;
                                                break;
                                            case 2: // block vertex 1 (A)
                                            case 3:
                                                aux.additionalData = getBlockLightAverage_(blockLightColor_[x][y][z],
                                                    basicVec3(x + 1, y, z), basicVec3(x, y - 1, z), basicVec3(x + 1, y - 1, z));
                                                aux.lightExtraData.x = 1;
                                                aux.lightExtraData.y = 0;
                                                break;
                                            case 5: // block vertex 2 (D)
                                                aux.additionalData = getBlockLightAverage_(blockLightColor_[x][y][z],
                                                    basicVec3(x + 1, y, z), basicVec3(x, y + 1, z), basicVec3(x + 1, y + 1, z));
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
                                                aux.additionalData = getBlockLightAverage_(blockLightColor_[x][y][z],
                                                    basicVec3(x + 1, y, z), basicVec3(x, y - 1, z), basicVec3(x + 1, y - 1, z));
                                                aux.lightExtraData.x = 0;
                                                aux.lightExtraData.y = 0;
                                                break;
                                            case 1: // block vertex 6 (C)
                                            case 4:
                                                aux.additionalData = getBlockLightAverage_(blockLightColor_[x][y][z],
                                                    basicVec3(x + 1, y, z), basicVec3(x, y + 1, z), basicVec3(x + 1, y + 1, z));
                                                aux.lightExtraData.x = 0;
                                                aux.lightExtraData.y = 1;
                                                break;
                                            case 2: // block vertex 4 (A)
                                            case 3:
                                                aux.additionalData = getBlockLightAverage_(blockLightColor_[x][y][z],
                                                    basicVec3(x - 1, y, z), basicVec3(x, y - 1, z), basicVec3(x - 1, y - 1, z));
                                                aux.lightExtraData.x = 1;
                                                aux.lightExtraData.y = 0;
                                                break;
                                            case 5: // block vertex 7 (D)
                                                aux.additionalData = getBlockLightAverage_(blockLightColor_[x][y][z],
                                                    basicVec3(x - 1, y, z), basicVec3(x, y + 1, z), basicVec3(x - 1, y + 1, z));
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
                                    aux.lightExtraData.z = bNeighbor->getMaterialIndex();

                                    switch (vertex)
                                    {
                                    case 0: // block vertex 4 (B)
                                        aux.additionalData = getBlockLightAverage_(blockLightColor_[CHUNK_SIZE_LIMIT][y][z],
                                            basicVec3(CHUNK_SIZE_LIMIT, y, z + 1), basicVec3(CHUNK_SIZE_LIMIT, y - 1, z), basicVec3(CHUNK_SIZE_LIMIT, y - 1, z + 1));
                                        aux.lightExtraData.x = 0;
                                        aux.lightExtraData.y = 0;
                                        break;
                                    case 1: // block vertex 7 (C)
                                    case 4:
                                        aux.additionalData = getBlockLightAverage_(blockLightColor_[CHUNK_SIZE_LIMIT][y][z],
                                            basicVec3(CHUNK_SIZE_LIMIT, y, z + 1), basicVec3(CHUNK_SIZE_LIMIT, y + 1, z), basicVec3(CHUNK_SIZE_LIMIT, y + 1, z + 1));
                                        aux.lightExtraData.x = 0;
                                        aux.lightExtraData.y = 1;
                                        break;
                                    case 2: // block vertex 0 (A)
                                    case 3:
                                        aux.additionalData = getBlockLightAverage_(blockLightColor_[CHUNK_SIZE_LIMIT][y][z],
                                            basicVec3(CHUNK_SIZE_LIMIT, y, z - 1), basicVec3(CHUNK_SIZE_LIMIT, y - 1, z), basicVec3(CHUNK_SIZE_LIMIT, y - 1, z - 1));
                                        aux.lightExtraData.x = 1;
                                        aux.lightExtraData.y = 0;
                                        break;
                                    case 5: // block vertex 3 (D)
                                        aux.additionalData = getBlockLightAverage_(blockLightColor_[CHUNK_SIZE_LIMIT][y][z],
                                            basicVec3(CHUNK_SIZE_LIMIT, y, z - 1), basicVec3(CHUNK_SIZE_LIMIT, y + 1, z), basicVec3(CHUNK_SIZE_LIMIT, y + 1, z - 1));
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

                    }

            }

            if (nTotalBlocksMinusX_) {

                for (y = 0; y < CHUNK_SIZE; y++)
                    for (z = 0; z < CHUNK_SIZE; z++) {

                        localID = blocksLocalIDs_[0][y][z];
                        const block& b = localID ? block::getBlockC(palette_.getT2(localID)) : block::emptyBlock();

                        if (b.opacity() <= blockOpacity::TRANSLUCENTBLOCK && (neighborLocalID = blocksLocalIDs_[-1][y][z])) {

                            bNeighbor = &block::getBlockC(palette_.getT2(neighborLocalID));

                            if (b != *bNeighbor) {

                                chunkModel = (bNeighbor->opacity() == blockOpacity::TRANSLUCENTBLOCK) ? &renderingData_.translucentVertices : &renderingData_.vertices;

                                // Create the face's vertices for face x+.
                                for (int vertex = 0; vertex < blockTriangles_->operator[](0).size(); vertex++) {

                                    aux.positions[0] = (chunkPos_.x - 1) * CHUNK_SIZE + (16 - 1) + blockVertices_->operator[](blockTriangles_->operator[](5)[vertex]).positions[0];
                                    aux.positions[1] = chunkPos_.y * CHUNK_SIZE + y + blockVertices_->operator[](blockTriangles_->operator[](5)[vertex]).positions[1];
                                    aux.positions[2] = chunkPos_.z * CHUNK_SIZE + z + blockVertices_->operator[](blockTriangles_->operator[](5)[vertex]).positions[2];
                                    aux.lightExtraData.z = bNeighbor->getMaterialIndex();

                                    switch (vertex)
                                    {
                                    case 0: // block vertex 1 (B)
                                        aux.additionalData = getBlockLightAverage_(blockLightColor_[0][y][z],
                                            basicVec3(0, y, z - 1), basicVec3(0, y - 1, z), basicVec3(0, y - 1, z - 1));
                                        aux.lightExtraData.x = 0;
                                        aux.lightExtraData.y = 0;
                                        break;
                                    case 1: // block vertex 2 (C)
                                    case 4:
                                        aux.additionalData = getBlockLightAverage_(blockLightColor_[0][y][z],
                                            basicVec3(0, y, z - 1), basicVec3(0, y + 1, z), basicVec3(0, y + 1, z - 1));
                                        aux.lightExtraData.x = 0;
                                        aux.lightExtraData.y = 1;
                                        break;
                                    case 2: // block vertex 5 (A)
                                    case 3:
                                        aux.additionalData = getBlockLightAverage_(blockLightColor_[0][y][z],
                                            basicVec3(0, y, z + 1), basicVec3(0, y - 1, z), basicVec3(0, y - 1, z + 1));
                                        aux.lightExtraData.x = 1;
                                        aux.lightExtraData.y = 0;
                                        break;
                                    case 5: // block vertex 6 (D)
                                        aux.additionalData = getBlockLightAverage_(blockLightColor_[0][y][z],
                                            basicVec3(0, y, z + 1), basicVec3(0, y + 1, z), basicVec3(0, y + 1, z + 1));
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
                                    aux.lightExtraData.z = bNeighbor->getMaterialIndex();

                                    switch (vertex)
                                    {
                                    case 0: // block vertex 1 (B)
                                        aux.additionalData = getBlockLightAverage_(blockLightColor_[x][CHUNK_SIZE_LIMIT][z],
                                            basicVec3(x + 1, CHUNK_SIZE_LIMIT, z), basicVec3(x, CHUNK_SIZE_LIMIT, z - 1), basicVec3(x + 1, CHUNK_SIZE_LIMIT, z - 1));
                                        aux.lightExtraData.x = 0;
                                        aux.lightExtraData.y = 0;
                                        break;
                                    case 1: // block vertex 5 (C)
                                    case 4:
                                        aux.additionalData = getBlockLightAverage_(blockLightColor_[x][CHUNK_SIZE_LIMIT][z],
                                            basicVec3(x + 1, CHUNK_SIZE_LIMIT, z), basicVec3(x, CHUNK_SIZE_LIMIT, z + 1), basicVec3(x + 1, CHUNK_SIZE_LIMIT, z + 1));
                                        aux.lightExtraData.x = 0;
                                        aux.lightExtraData.y = 1;
                                        break;
                                    case 2: // block vertex 0 (A)
                                    case 3:
                                        aux.additionalData = getBlockLightAverage_(blockLightColor_[x][CHUNK_SIZE_LIMIT][z],
                                            basicVec3(x - 1, CHUNK_SIZE_LIMIT, z), basicVec3(x, CHUNK_SIZE_LIMIT, z - 1), basicVec3(x - 1, CHUNK_SIZE_LIMIT, z - 1));
                                        aux.lightExtraData.x = 1;
                                        aux.lightExtraData.y = 0;
                                        break;
                                    case 5: // block vertex 4 (D)
                                        aux.additionalData = getBlockLightAverage_(blockLightColor_[x][CHUNK_SIZE_LIMIT][z],
                                            basicVec3(x - 1, CHUNK_SIZE_LIMIT, z), basicVec3(x, CHUNK_SIZE_LIMIT, z + 1), basicVec3(x - 1, CHUNK_SIZE_LIMIT, z + 1));
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
                                    aux.lightExtraData.z = bNeighbor->getMaterialIndex();

                                    switch (vertex)
                                    {
                                    case 0: // block vertex 3 (B)
                                        aux.additionalData = getBlockLightAverage_(blockLightColor_[x][0][z],
                                            basicVec3(x - 1, 0, z), basicVec3(x, 0, z - 1), basicVec3(x - 1, 0, z - 1));
                                        aux.lightExtraData.x = 0;
                                        aux.lightExtraData.y = 0;
                                        break;
                                    case 1: // block vertex 7 (C)
                                    case 4:
                                        aux.additionalData = getBlockLightAverage_(blockLightColor_[x][0][z],
                                            basicVec3(x - 1, 0, z), basicVec3(x, 0, z + 1), basicVec3(x - 1, 0, z + 1));
                                        aux.lightExtraData.x = 0;
                                        aux.lightExtraData.y = 1;
                                        break;
                                    case 2: // block vertex 2 (A)
                                    case 3:
                                        aux.additionalData = getBlockLightAverage_(blockLightColor_[x][0][z],
                                            basicVec3(x + 1, 0, z), basicVec3(x, 0, z - 1), basicVec3(x + 1, 0, z - 1));
                                        aux.lightExtraData.x = 1;
                                        aux.lightExtraData.y = 0;
                                        break;
                                    case 5: // block vertex 6 (D)
                                        aux.additionalData = getBlockLightAverage_(blockLightColor_[x][0][z],
                                            basicVec3(x + 1, 0, z), basicVec3(x, 0, z + 1), basicVec3(x + 1, 0, z + 1));
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
                                    aux.lightExtraData.z = bNeighbor->getMaterialIndex();

                                    switch (vertex)
                                    {
                                    case 0: // block vertex 0 (B)
                                        aux.additionalData = getBlockLightAverage_(blockLightColor_[x][y][CHUNK_SIZE_LIMIT],
                                            basicVec3(x - 1, y, CHUNK_SIZE_LIMIT), basicVec3(x, y - 1, CHUNK_SIZE_LIMIT), basicVec3(x - 1, y - 1, CHUNK_SIZE_LIMIT));
                                        aux.lightExtraData.x = 0;
                                        aux.lightExtraData.y = 0;
                                        break;
                                    case 1: // block vertex 3 (C)
                                    case 4:
                                        aux.additionalData = getBlockLightAverage_(blockLightColor_[x][y][CHUNK_SIZE_LIMIT],
                                            basicVec3(x - 1, y, CHUNK_SIZE_LIMIT), basicVec3(x, y + 1, CHUNK_SIZE_LIMIT), basicVec3(x - 1, y + 1, CHUNK_SIZE_LIMIT));
                                        aux.lightExtraData.x = 0;
                                        aux.lightExtraData.y = 1;
                                        break;
                                    case 2: // block vertex 1 (A)
                                    case 3:
                                        aux.additionalData = getBlockLightAverage_(blockLightColor_[x][y][CHUNK_SIZE_LIMIT],
                                            basicVec3(x + 1, y, CHUNK_SIZE_LIMIT), basicVec3(x, y - 1, CHUNK_SIZE_LIMIT), basicVec3(x + 1, y - 1, CHUNK_SIZE_LIMIT));
                                        aux.lightExtraData.x = 1;
                                        aux.lightExtraData.y = 0;
                                        break;
                                    case 5: // block vertex 2 (D)
                                        aux.additionalData = getBlockLightAverage_(blockLightColor_[x][y][CHUNK_SIZE_LIMIT],
                                            basicVec3(x + 1, y, CHUNK_SIZE_LIMIT), basicVec3(x, y + 1, CHUNK_SIZE_LIMIT), basicVec3(x + 1, y + 1, CHUNK_SIZE_LIMIT));
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
                                    aux.lightExtraData.z = bNeighbor->getMaterialIndex();

                                    switch (vertex)
                                    {
                                    case 0: // block vertex 5 (B)
                                        aux.additionalData = getBlockLightAverage_(blockLightColor_[x][y][0],
                                            basicVec3(x + 1, y, 0), basicVec3(x, y - 1, 0), basicVec3(x + 1, y - 1, 0));
                                        aux.lightExtraData.x = 0;
                                        aux.lightExtraData.y = 0;
                                        break;
                                    case 1: // block vertex 6 (C)
                                    case 4:
                                        aux.additionalData = getBlockLightAverage_(blockLightColor_[x][y][0],
                                            basicVec3(x + 1, y, 0), basicVec3(x, y + 1, 0), basicVec3(x + 1, y + 1, 0));
                                        aux.lightExtraData.x = 0;
                                        aux.lightExtraData.y = 1;
                                        break;
                                    case 2: // block vertex 4 (A)
                                    case 3:
                                        aux.additionalData = getBlockLightAverage_(blockLightColor_[x][y][0],
                                            basicVec3(x - 1, y, 0), basicVec3(x, y - 1, 0), basicVec3(x - 1, y - 1, 0));
                                        aux.lightExtraData.x = 1;
                                        aux.lightExtraData.y = 0;
                                        break;
                                    case 5: // block vertex 7 (D)
                                        aux.additionalData = getBlockLightAverage_(blockLightColor_[x][y][0],
                                            basicVec3(x - 1, y, 0), basicVec3(x, y + 1, 0), basicVec3(x - 1, y + 1, 0));
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

                    }

            }

            // Read chunk data section ends.
            blocksMutex_.unlock_shared();

        }
         
        {
            std::unique_lock<std::recursive_mutex> lock(loadStatusMutex_);
            if (loadStatus_ < chunkLoadStatus::MESHED)
                loadStatus_ = chunkLoadStatus::MESHED;
        }
        
        renderingData_.totalSize = renderingData_.vertices.size() + renderingData_.translucentVertices.size();
        return renderingData_.totalSize;

    }

    void chunk::clearBlockLight() {

        isOpaque_.clear();
        floodPointLightPositions_.clear();
        blockLightColor_.clear();
        blockLightLevel_.clear();

    }

    void chunk::addBlockLight(char x, char y, char z, const basicVec4& color, char intensity, bool markToBeRemeshed) {

        float modifier = intensity / 8.0f;
        float xColor = blockLightColor_[x][y][z].x + color.x * modifier > 127.0f ? 127.0f : blockLightColor_[x][y][z].x + color.x * modifier;
        float yColor = blockLightColor_[x][y][z].y + color.y * modifier > 127.0f ? 127.0f : blockLightColor_[x][y][z].y + color.y * modifier;
        float zColor = blockLightColor_[x][y][z].z + color.z * modifier > 127.0f ? 127.0f : blockLightColor_[x][y][z].z + color.z * modifier;

        blockLightColor_[x][y][z].x = xColor;
        blockLightColor_[x][y][z].y = yColor;
        blockLightColor_[x][y][z].z = zColor;
        blockLightLevel_[x][y][z] = intensity;

        if (markToBeRemeshed)
            needsRemesh_ = true;
    }

    void chunk::applyBlockLight(char x, char y, char z, const basicVec4& color, char intensity, bool markToBeRemeshed) {

        blockLightColor_[x][y][z].x = color.x * (intensity / 8.0f);
        blockLightColor_[x][y][z].y = color.y * (intensity / 8.0f);
        blockLightColor_[x][y][z].z = color.z * (intensity / 8.0f);
        blockLightLevel_[x][y][z] = intensity;

        if (markToBeRemeshed)
            needsRemesh_ = true;
    }

    void chunk::makeEmpty() {

        {
            std::unique_lock<std::recursive_mutex> lock(loadStatusMutex_);
            loadStatus_ = chunkLoadStatus::NOTLOADED;
        }

        blocksMutex_.lock();

        poraqui = false;
        poraqui2 = false;
        poraqui3 = false;
        poraqui4 = false;
        poraqui5 = false;
        poraqui6 = false;
        poraqui7 = false;
        poraqui8 = false;
        poraqui9 = false;
        poraqui10 = false;
        poraqui11 = false;
        poraqui12 = false;
        poraqui13 = false;
        poraqui14 = false;
        poraqui15 = chunkLoadStatus::NOTLOADED;
        poraqui16 = false;
        poraqui17 = false;
        poraqui18 = false;
        poraqui19 = false;
        poraqui20 = false;
        poraqui21 = false;
        poraqui22 = false;
        poraqui23 = false;
        poraqui24 = false;
        poraqui25 = false;
        poraqui26 = false;
        poraqui27 = false;
        poraqui28 = false;
        poraqui29 = false;
        wasMadeEmpty = true;

        palette_.clear();
        paletteCount_.clear();
        freeLocalIDs_.clear();
        clearBlockLight();
        blocksLocalIDs_.clear();

        modified_ = false;
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

        needsRemesh_ = false;
        loadedFromDisk_ = false;

        owners_.clear();
        unloadWhenNoOwners_ = false;

        chunkPos_ = vec3Zero;

        renderingDataMutex_.lock();
        renderingData_ = chunkRenderingData();
        renderingDataMutex_.unlock();

        {
            std::unique_lock<std::recursive_mutex> lock(ownedChunksMutex_);
            ownedChunks_.clear();
        }

        blocksMutex_.unlock();

    }

    void chunk::onUnloadAsFrontier() {

        // Check for the neighbors of this recently unloaded frontier chunk
        chunkManager::onUnloadAsFrontier(ivec3{ chunkPos_.x + 1, chunkPos_.y, chunkPos_.z });
        chunkManager::onUnloadAsFrontier(ivec3{ chunkPos_.x - 1, chunkPos_.y, chunkPos_.z });
        chunkManager::onUnloadAsFrontier(ivec3{ chunkPos_.x, chunkPos_.y + 1, chunkPos_.z });
        chunkManager::onUnloadAsFrontier(ivec3{ chunkPos_.x, chunkPos_.y - 1, chunkPos_.z });
        chunkManager::onUnloadAsFrontier(ivec3{ chunkPos_.x, chunkPos_.y, chunkPos_.z + 1 });
        chunkManager::onUnloadAsFrontier(ivec3{ chunkPos_.x, chunkPos_.y, chunkPos_.z - 1 });

    }

    void chunk::postGenPass() {



    }

    void chunk::addOwner(const ivec3& chunkPos) {

        owners_.insert(chunkPos);

    }

    void chunk::removeOwner(const ivec3& chunkPos) {

        owners_.erase(chunkPos);

    }

    void chunk::unloadWhenNoOwners(bool value) {

        unloadWhenNoOwners_ = value;

    }

    void chunk::getAndOwnChunks(const lightDataToSet& data) {

        ivec3 chunkOffset = ivec3Zero;
        ivec3 chunkCoords = ivec3Zero;

        {
            std::unique_lock<std::recursive_mutex> lock(chunkManager::chunksMutex());
            std::unique_lock<std::recursive_mutex> lock2(ownedChunksMutex_);
            
            for (auto it = data.cbegin(); it != data.cend(); it++) {

                chunk* c = chunkManager::getChunk(it->first);
                ownedChunks_[chunkCoords] = c;
                if (c)
                    c->addOwner(chunkPos_);

            }

        }

    }

    void chunk::disownChunks() {

        {
        
            std::unique_lock<std::recursive_mutex> lock(chunkManager::chunksMutex());
            std::unique_lock<std::recursive_mutex> lock2(ownedChunksMutex_);
            
            for (auto it = ownedChunks_.cbegin(); it != ownedChunks_.cend(); it++)
                if (chunk* c = it->second)
                    chunkManager::removeOwner(*c, chunkPos_);
            ownedChunks_.clear();
        
        }

    }

    void chunk::reset() {

        blockVertices_ = nullptr;
        blockTriangles_ = nullptr;
        blockNormals_ = nullptr;

        initialised_ = false;

    }


    bool chunk::placeNewBlock_(unsigned short& actualLocalID, const block& newBlock) {

        unsigned int newGlobalID = newBlock.intID(),
            oldGlobalID = actualLocalID ? palette_.getT2(actualLocalID) : 0;
        const bool blockChanged = oldGlobalID != newGlobalID;
        needsRemesh_ = needsRemesh_ || blockChanged;

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

        return blockChanged;

    }

    basicVec4 chunk::getBlockLightAverage_(const basicVec4& blockLightOwn,
        const basicVec3& blockLightCoords1, const basicVec3& blockLightCoords2, const basicVec3& blockLightCoords3) {

        const char AOLight = 0;

        float x = (blockLightOwn.x +
            (isOpaque_[blockLightCoords1.x][blockLightCoords1.y][blockLightCoords1.z] ? AOLight :
                blockLightColor_[blockLightCoords1.x][blockLightCoords1.y][blockLightCoords1.z].x) +
            (isOpaque_[blockLightCoords2.x][blockLightCoords2.y][blockLightCoords2.z] ? AOLight :
                blockLightColor_[blockLightCoords2.x][blockLightCoords2.y][blockLightCoords2.z].x) +
            (isOpaque_[blockLightCoords3.x][blockLightCoords3.y][blockLightCoords3.z] ? AOLight :
                blockLightColor_[blockLightCoords3.x][blockLightCoords3.y][blockLightCoords3.z].x)) / 4;
        float y = (blockLightOwn.y +
            (isOpaque_[blockLightCoords1.x][blockLightCoords1.y][blockLightCoords1.z] ? AOLight :
                blockLightColor_[blockLightCoords1.x][blockLightCoords1.y][blockLightCoords1.z].y) +
            (isOpaque_[blockLightCoords2.x][blockLightCoords2.y][blockLightCoords2.z] ? AOLight :
                blockLightColor_[blockLightCoords2.x][blockLightCoords2.y][blockLightCoords2.z].y) +
            (isOpaque_[blockLightCoords3.x][blockLightCoords3.y][blockLightCoords3.z] ? AOLight :
                blockLightColor_[blockLightCoords3.x][blockLightCoords3.y][blockLightCoords3.z].y)) / 4;
        float z = (blockLightOwn.z +
            (isOpaque_[blockLightCoords1.x][blockLightCoords1.y][blockLightCoords1.z] ? AOLight :
                blockLightColor_[blockLightCoords1.x][blockLightCoords1.y][blockLightCoords1.z].z) +
            (isOpaque_[blockLightCoords2.x][blockLightCoords2.y][blockLightCoords2.z] ? AOLight :
                blockLightColor_[blockLightCoords2.x][blockLightCoords2.y][blockLightCoords2.z].z) +
            (isOpaque_[blockLightCoords3.x][blockLightCoords3.y][blockLightCoords3.z] ? AOLight :
                blockLightColor_[blockLightCoords3.x][blockLightCoords3.y][blockLightCoords3.z].z)) / 4;


        return basicVec4((char)x, (char)y, (char)z, 127);

    }

    void chunk::getAndOwnChunksWithOffset_(int negOffsetX, int negOffsetY, int negOffsetZ, int offsetX, int offsetY, int offsetZ, 
        bool relativeCoords) {

        ivec3 chunkOffset = vec3Zero;
        ivec3 chunkCoords = vec3Zero;

        {
            std::unique_lock<std::recursive_mutex> lock(chunkManager::chunksMutex());
            std::unique_lock<std::recursive_mutex> lock2(ownedChunksMutex_);
            for (int i = negOffsetX; i <= offsetX; i++)
                for (int j = negOffsetY; j <= offsetY; j++)
                    for (int k = negOffsetZ; k <= offsetZ; k++) {

                        chunkOffset.x = i;
                        chunkOffset.y = j;
                        chunkOffset.z = k;
                        chunkCoords = chunkPos_ + chunkOffset;
                        chunk* c = chunkManager::getChunk(chunkCoords);
                        if (!c)
                            c = chunkManager::registerChunk(chunkCoords, false, true);
                        ownedChunks_[relativeCoords ? chunkOffset : chunkCoords] = c;
                        c->addOwner(chunkPos_);

                    }
        }

    }


    // 'chunkEvent' class.

    void chunkEvent::notify(const ivec2& chunkPosXZ) {

        chunkPosXZ_ = chunkPosXZ;
        event::notify();

    }


    // 'chunkManager' class.

    bool chunkManager::initialised_ = false;
    bool chunkManager::chunkJobSystemOverloaded_ = false;
    int chunkManager::nChunksToCompute_ = 0;

    std::atomic<bool> chunkManager::clearChunksFlag_ = false;

    std::atomic<bool> chunkManager::waitInitialTerrainLoaded_ = true;

    chunksMap chunkManager::clientChunks_;
    chunksMap chunkManager::simulatedChunks_;

    std::unordered_set<ivec3>* chunkManager::chunkMeshes_ = nullptr;

    std::unordered_map<ivec3, chunkVBOop>* chunkManager::chunkVBOopsWrite_ = nullptr;
    std::unordered_map<ivec3, chunkVBOop>* chunkManager::chunkVBOopsPriorityWrite_ = nullptr;
    std::unordered_map<ivec3, chunkVBOop>* chunkManager::chunkVBOopsRead_ = nullptr;
    std::unordered_map<ivec3, chunkVBOop>* chunkManager::chunkVBOopsPriorityRead_ = nullptr;

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

    std::unordered_map<unsigned int, std::unordered_map<ivec3, bool>> chunkManager::AIChunkAvailable_;
    std::unordered_map<unsigned int, std::unordered_map<ivec3, chunk*>> chunkManager::AIagentChunks_;
    unsigned int chunkManager::selectedAIWorld_ = 0;
    bool chunkManager::originalWorldAccess_ = true;

    ivec3 chunkManager::playerChunkPosCopy_;
    std::list<ivec3> chunkManager::frontierChunks_;
    std::unordered_map<ivec3, std::list<ivec3>::iterator> chunkManager::frontierChunksSet_;
    std::list<ivec3>::iterator chunkManager::frontierIt_;

    chunkManager::closestChunk chunkManager::closestChunk_;

    threadPool* chunkManager::chunkTasks_ = nullptr;
    threadPool* chunkManager::priorityChunkTasks_ = nullptr;

    atomicRecyclingPool<job>* chunkManager::loadChunkJobs_;
    atomicRecyclingPool<chunk> chunkManager::chunksPool_;

    std::mutex chunkManager::lightTickFunctionsMutex_;
    atomicRecyclingPool<job>* chunkManager::lightJobs_ = nullptr;
    std::list<tickFunc> chunkManager::pendingLightJobs_;
    threadPool* chunkManager::lightTasks_ = nullptr;

    chunkEvent chunkManager::onChunkLoad_("On chunk load");
    chunkEvent chunkManager::onChunkUnload_("On chunk unload");

    std::mutex chunkManager::chunkNeighborsInfoMutex_;
    std::unordered_map<ivec3, std::shared_ptr<neighborsInfo>> chunkManager::chunkNeighborsInfo_;

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

            chunkMeshes_ = new std::unordered_set<ivec3>;

            chunkVBOopsWrite_ = new std::unordered_map<ivec3, chunkVBOop>;
            chunkVBOopsPriorityWrite_ = new std::unordered_map<ivec3, chunkVBOop>;
            chunkVBOopsRead_ = new std::unordered_map<ivec3, chunkVBOop>;
            chunkVBOopsPriorityRead_ = new std::unordered_map<ivec3, chunkVBOop>;

            chunkTasks_ = new threadPool(game::getSettings().maxChunkhreads());
            priorityChunkTasks_ = new threadPool(2);
            loadChunkJobs_ = new atomicRecyclingPool<job>(game::getSettings().maxChunkhreads());
            loadChunkJobs_->setAllFreeOnClear(true);
            chunksPool_.setAllFreeOnClear(false);
            vbo_ = static_cast<chunkVertexBuffer*>(graphics::pVbo("chunks"));
            vbo_->bind();
            vbo_->prepareDynamic(1024LL * 1024 * 1024 * 2); // 1024^3 bytes = 1GB.

            lightJobs_ = new atomicRecyclingPool<job>(1); // Only one thread for processing block lighting updates.
            lightJobs_->setAllFreeOnClear(true);
            lightTasks_ = new threadPool(1);

            clearChunksFlag_ = false;

            initialised_ = true;

        }

    }

    void chunkManager::updatePriorityReadChunkMeshes() {
        
        chunkVBOopsPriorityWrite_->clear();

        chunk* c = nullptr;
        std::unique_lock<std::mutex> priorityUpdatesLock(priorityUpdatesRemainingMutex_);
        for (auto it = priorityNewChunkMeshes_.begin(); it != priorityNewChunkMeshes_.end();) {

            c = *it;
            c->lockSharedRenderingDataMutex();
            chunkRenderingData& data = c->renderingData();
            if (data.totalSize)
                chunkVBOopsPriorityWrite_->operator[](c->chunkPos()) = chunkVBOop(VBOop::PUSH, data);
            c->unlockSharedRenderingDataMutex();
            it = priorityNewChunkMeshes_.erase(it);

        }

    }

    void chunkManager::updateReadChunkMeshes() {

        chunkVBOopsWrite_->clear(); 

        for (auto it = chunkMeshes_->begin(); it != chunkMeshes_->end();) {

            const ivec3& chunkPos = *it;
            chunk* c = getChunk_(chunkPos);
            if (!c) {

                chunkVBOopsWrite_->operator[](chunkPos) = chunkVBOop(VBOop::FREE);
                it = chunkMeshes_->erase(it);

            }
            else {
            
                ivec3 distancePlayer = chunkDistance(chunkPos, playerChunkPosCopy_);
                bool a = distancePlayer.x > nChunksToCompute_ + 10 ||
                    distancePlayer.z > nChunksToCompute_ + 10;
                if (a)
                    int b = 3 + 2; // HAY CHUNKS QUE NO SE DESCARGAN NUNCA PORQUE SE HAN QUEDADO EN SIMULATED CON OWNERS COLGANDO. PRUEBA A METER LOS OWNERS EN UN STD::MAP MEJOR.

                it++;
            
            }
                

        }

        chunk* c = nullptr;
        std::unique_lock<std::recursive_mutex> updatesLock(newChunkMeshesMutex_);
        for (auto it = newChunkMeshes_.begin(); it != newChunkMeshes_.end();) {

            c = *it;
            const ivec3& chunkPos = c->chunkPos();
            c->loadStatusMutex().lock();
            if (c->loadStatus() >= chunkLoadStatus::MESHED && chunkInRenderDistance(chunkPos, camera::cPlayerCamera()->chunkPos())) { // TODO. Is this condition truly necessary?
                c->loadStatusMutex().unlock();
                c->lockSharedRenderingDataMutex();

                chunkRenderingData& data = c->renderingData();
                if (data.totalSize) {

                    chunkMeshes_->insert(chunkPos);
                    chunkVBOopsWrite_->operator[](chunkPos) = chunkVBOop(VBOop::PUSH, data);
                    c->poraqui23 = true;

                }
                c->unlockSharedRenderingDataMutex();

                it = newChunkMeshes_.erase(it);

            }
            else {
            
                c->loadStatusMutex().unlock();
                it++;
            
            } 

        }

    }

    void chunkManager::swapChunkMeshesBuffers() {

        std::unordered_map<ivec3, chunkVBOop>* auxChunkVBOops = chunkVBOopsWrite_;
        chunkVBOopsWrite_ = chunkVBOopsRead_;
        chunkVBOopsRead_ = auxChunkVBOops;

    }

    void chunkManager::swapChunkMeshesPriorityBuffers() {

        std::unordered_map<ivec3, chunkVBOop>* auxVBOops = chunkVBOopsPriorityWrite_;
        chunkVBOopsPriorityWrite_ = chunkVBOopsPriorityRead_;
        chunkVBOopsPriorityRead_ = auxVBOops;

    }

    const block& chunkManager::getBlock(int posX, int posY, int posZ) {

        std::unique_lock<std::recursive_mutex> lock(chunksMutex_);
        return getBlockOGWorld_(posX, posY, posZ);

    }

    bool chunkManager::isChunkInWorld(const ivec3& chunkPos) {

        std::unique_lock<std::recursive_mutex> lock(chunksMutex_);
        return clientChunks_.find(chunkPos) != clientChunks_.cend();

    }

    chunkLoadStatus chunkManager::getChunkLoadStatus(const ivec3& chunkPos) {

        bool found = false;
        chunksMap::iterator it;

        {
            std::unique_lock<std::recursive_mutex> lock(chunksMutex_);
            it = clientChunks_.find(chunkPos);
            found = it != clientChunks_.cend();
        }

        if (found) {
        
            std::unique_lock<std::recursive_mutex> lock(it->second->loadStatusMutex());
            return it->second->loadStatus();
        
        }
        else
            return chunkLoadStatus::NOTLOADED;

    }

    bool chunkManager::isEmptyBlock(int posX, int posY, int posZ) {

        const block* selectedBlock;
        std::unique_lock<std::recursive_mutex> lock(chunksMutex_);

        selectedBlock = &getBlockOGWorld_(posX, posY, posZ);

        return selectedBlock->isEmptyBlock();

    }

    bool chunkManager::chunkInRenderDistance(const ivec3& chunkPos) {

        ivec3 distancePlayer = chunkDistance(chunkPos, playerChunkPosCopy_);
        return distancePlayer.x <= nChunksToCompute_ &&
            distancePlayer.z <= nChunksToCompute_ &&
            distancePlayer.y <= yChunksRange;

    }

    bool chunkManager::chunkInRenderDistance(const ivec3& chunkPos, const vec3& playerPos) {

        ivec3 distancePlayer = chunkDistance(chunkPos, playerPos);
        return distancePlayer.x <= nChunksToCompute_ &&
            distancePlayer.z <= nChunksToCompute_ &&
            distancePlayer.y <= yChunksRange;

    }

    ivec3 chunkManager::chunkDistanceToPlayer(const ivec3& chunkPos) {

        const ivec3& playerPos = player::getCamera().chunkPos();
        return ivec3{ (float)std::abs(chunkPos.x - playerPos.x),
                     (float)std::abs(chunkPos.y - playerPos.y),
                     (float)std::abs(chunkPos.z - playerPos.z) };

    }

    ivec3 chunkManager::chunkDistance(const ivec3& chunkPos1, const ivec3& chunkPos2) {

        ivec3 signedDistance = chunkSignedDistance(chunkPos1, chunkPos2);
        return abs(signedDistance);

    }

    ivec3 chunkManager::chunkSignedDistance(const ivec3& chunkPos1, const ivec3& chunkPos2) {

        return ivec3{ chunkPos1.x - chunkPos2.x,
                     chunkPos1.y - chunkPos2.y,
                     chunkPos1.z - chunkPos2.z };

    }

    double chunkManager::distanceToPlayer(const ivec3& chunkPos) {

        const ivec3& playerPos = player::getCamera().chunkPos();
        double distanceX = playerPos.x - (chunkPos.x + 0.5) * CHUNK_SIZE;
        double distanceZ = playerPos.y - (chunkPos.y + 0.5) * CHUNK_SIZE;
        double distanceY = playerPos.z - (chunkPos.z + 0.5) * CHUNK_SIZE;
        return distanceX * distanceX + distanceZ * distanceZ + distanceY * distanceY;

    }

    neighborsInfo* chunkManager::getChunkNeighborInfo(const ivec3& chunkPos) {

        chunkNeighborsInfoMutex_.lock();
        neighborsInfo* neighborsInfoPtr = chunkNeighborsInfo_[chunkPos].get();
        chunkNeighborsInfoMutex_.unlock();
        return neighborsInfoPtr;

    }

    void chunkManager::setNChunksToCompute(unsigned int nChunksToCompute) {

        engineMode mode = game::selectedEngineMode();
        if (mode == engineMode::INITLEVEL || mode == engineMode::EDITLEVEL)
            nChunksToCompute_ = nChunksToCompute;
        else
            logger::errorLog("Cannot change the number of chunks to compute in the current engine mode " + std::to_string((unsigned int)mode));

    }

    const block& chunkManager::setBlock(int x, int y, int z, const block& blockID) {

        ivec3 chunkPos = getChunkCoords(x, y, z);
        std::unique_lock<std::recursive_mutex> lock(chunksMutex_);
        auto it = clientChunks_.find(chunkPos);
        if (it == clientChunks_.cend())
            logger::errorLog("Chunk " + std::to_string(chunkPos.x) + "|" + std::to_string(chunkPos.y) + "|" + std::to_string(chunkPos.z) + " does not exist");
        else
            return it->second->setBlock(getChunkRelCoords(x, y, z), blockID);

    }

    chunk* chunkManager::neighborMinusX(const ivec3& chunkPos) {

        ivec3 neighborChunkPos{ chunkPos.x - 1, chunkPos.y, chunkPos.z };
        std::unique_lock<std::recursive_mutex> lock(chunksMutex_);
        auto it = clientChunks_.find(neighborChunkPos);
        if (it != clientChunks_.end())
            return it->second;
        else
            return nullptr;

    }

    chunk* chunkManager::neighborPlusX(const ivec3& chunkPos) {

        ivec3 neighborChunkPos{ chunkPos.x + 1, chunkPos.y, chunkPos.z };
        std::unique_lock<std::recursive_mutex> lock(chunksMutex_);
        auto it = clientChunks_.find(neighborChunkPos);
        if (it != clientChunks_.end())
            return it->second;
        else
            return nullptr;

    }

    chunk* chunkManager::neighborMinusY(const ivec3& chunkPos) {

        ivec3 neighborChunkPos{ chunkPos.x, chunkPos.y - 1, chunkPos.z };
        std::unique_lock<std::recursive_mutex> lock(chunksMutex_);
        auto it = clientChunks_.find(neighborChunkPos);
        if (it != clientChunks_.end())
            return it->second;
        else
            return nullptr;

    }

    chunk* chunkManager::neighborPlusY(const ivec3& chunkPos) {

        ivec3 neighborChunkPos{ chunkPos.x, chunkPos.y + 1, chunkPos.z };
        std::unique_lock<std::recursive_mutex> lock(chunksMutex_);
        auto it = clientChunks_.find(neighborChunkPos);
        if (it != clientChunks_.end())
            return it->second;
        else
            return nullptr;

    }

    chunk* chunkManager::neighborMinusZ(const ivec3& chunkPos) {

        ivec3 neighborChunkPos{ chunkPos.x, chunkPos.y, chunkPos.z - 1 };
        std::unique_lock<std::recursive_mutex> lock(chunksMutex_);
        auto it = clientChunks_.find(neighborChunkPos);
        if (it != clientChunks_.end())
            return it->second;
        else
            return nullptr;

    }

    chunk* chunkManager::neighborPlusZ(const ivec3& chunkPos) {

        ivec3 neighborChunkPos{ chunkPos.x, chunkPos.y, chunkPos.z + 1 };
        std::unique_lock<std::recursive_mutex> lock(chunksMutex_);
        auto it = clientChunks_.find(neighborChunkPos);
        if (it != clientChunks_.end())
            return it->second;
        else
            return nullptr;

    }

    void chunkManager::waitInitialTerrainLoaded() {

        std::unique_lock<std::mutex> lock(loadingTerrainMutex_);
        while (waitInitialTerrainLoaded_)
            loadingTerrainCV_.wait(lock);

    }

    void chunkManager::unloadFrontierChunk(const ivec3& chunkPos) {

        std::unique_lock<std::recursive_mutex> lock(chunksMutex_);
        chunk* c = nullptr;
        auto it = clientChunks_.find(chunkPos);
        if (it != clientChunks_.cend())
            c = it->second;
        else {
        
            auto it2 = simulatedChunks_.find(chunkPos);
            if (it2 != simulatedChunks_.cend())
                c = it2->second;
        
        }
        
        if (c)
        {
            c->poraqui17 = true;

            // Check if the chunk's neighbors become frontier chunks after it is unloaded.
            c->onUnloadAsFrontier();

            frontierChunks_.erase(frontierChunksSet_.at(chunkPos));
            frontierChunksSet_.erase(chunkPos);

            onChunkUnload_.notify(chunkPos.x, chunkPos.z);
            onChunkUnload_.notify(chunkPos.x + 1, chunkPos.z);
            onChunkUnload_.notify(chunkPos.x - 1, chunkPos.z);
            onChunkUnload_.notify(chunkPos.x, chunkPos.z + 1);
            onChunkUnload_.notify(chunkPos.x, chunkPos.z - 1);

            clientChunks_.erase(chunkPos);
            if (c->isOwned()) {

                simulatedChunks_[chunkPos] = c;
                c->disownChunks();
                c->unloadWhenNoOwners(true);
                c->poraqui18 = true;

            }
            else {
            
                c->poraqui19 = true;
                decreaseNeighborInfoPassCounter_(*c);
                issueChunkJob(chunkJobType::UNLOADANDSAVE, c, true);
            
            }

        }

    }

    void chunkManager::onUnloadAsFrontier(const ivec3& chunkPos) {

        if (!frontierChunksSet_.contains(chunkPos) && chunkExists_(chunkPos) != chunkExistence::NOEXISTS) {
        
            if (clientChunks_.contains(chunkPos))
                clientChunks_[chunkPos]->poraqui26 = true;
            frontierChunksSet_[chunkPos] = frontierChunks_.insert(frontierChunks_.end(), chunkPos);
        
        }  

    }

    void chunkManager::addFrontier(chunk* chunk) {

        const ivec3& chunkPos = chunk->chunkPos();
        if (!frontierChunksSet_.contains(chunkPos)) {
        
            frontierChunksSet_[chunkPos] = frontierChunks_.insert(frontierChunks_.end(), chunkPos);

            onChunkLoad_.notify(chunkPos.x, chunkPos.z);
            onChunkLoad_.notify(chunkPos.x + 1, chunkPos.z);
            onChunkLoad_.notify(chunkPos.x - 1, chunkPos.z);
            onChunkLoad_.notify(chunkPos.x, chunkPos.z + 1);
            onChunkLoad_.notify(chunkPos.x, chunkPos.z - 1);
        
        }

    }

    void chunkManager::remesh(chunk* c, bool isPriorityUpdate, bool forceRemesh) {

        pushNewChunkMesh_(isPriorityUpdate, c, c->renewMesh(forceRemesh));

    }

    void chunkManager::renewMesh(const ivec3& chunkPos, bool isPriorityUpdate) {

        chunksMap::const_iterator it;
        bool found = false;
        chunk* c = nullptr;
        {
            std::unique_lock<std::recursive_mutex> lock(chunksMutex_);
            it = clientChunks_.find(chunkPos);
            if (found = (it == clientChunks_.cend()))
                c = it->second;
        }
        
        if (found)
            logger::errorLog("The chunk " + std::to_string(chunkPos) + " is not registered");
        else
            remesh(c, isPriorityUpdate);

    }

    chunk* chunkManager::getChunkByChunkPos(int x, int y, int z) {

        return getChunk_({ x,y,z });

    }

    chunk* chunkManager::getChunkByRealPos(float x, float y, float z) {

        return getChunk_(getChunkCoords(x, y, z));

    }

    chunk* chunkManager::addOwner(const ivec3& chunkPos, const ivec3& ownerChunkPos) {

        std::unique_lock<std::recursive_mutex> lock(chunksMutex_);
        chunk* c = getChunk_(chunkPos);
        if (c)
            c->addOwner(ownerChunkPos);

        return c;

    }

    void chunkManager::removeOwner(chunk& c, const ivec3& ownerChunkPos) {

        std::unique_lock<std::recursive_mutex> lock(chunksMutex_);
        if (c.isOwned()) {
            c.removeOwner(ownerChunkPos);
            if (!c.isOwned()) {

                const ivec3& chunkPos = c.chunkPos();
                if (simulatedChunks_.erase(chunkPos)) { // If chunk was simulated.
                
                    if (c.unloadWhenNoOwners()) { // And was told to unload when it has no owners, unload it.

                        decreaseNeighborInfoPassCounter_(c);
                        issueChunkJob(chunkJobType::UNLOADANDSAVE, &c, true);

                    }
                    else { // Else, pass it to common chunks again IF inside chunk rendering distance. Otherwise, unload it.

                        c.poraqui14 = true;
                        c.poraqui15 = c.loadStatus();

                        if (chunkInRenderDistance(chunkPos)) {

                            c.poraqui29 = true;
                            simulatedChunkToCommon_(&c);

                        }
                        else {
                        
                            decreaseNeighborInfoPassCounter_(c);
                            issueChunkJob(chunkJobType::UNLOADANDSAVE, &c, true);
                        
                        }

                    }
                
                }

            }
        
        }

    }

    std::shared_ptr<neighborsInfo> chunkManager::getOrCreateChunkNeighborInfo(const ivec3& chunkPos) {

        chunkNeighborsInfoMutex_.lock();
        std::shared_ptr<neighborsInfo>& neighborsInfoPtr = chunkNeighborsInfo_[chunkPos];
        if (!neighborsInfoPtr)
            neighborsInfoPtr = std::make_shared<neighborsInfo>();
        chunkNeighborsInfoMutex_.unlock();
        return neighborsInfoPtr;

    }

    void chunkManager::addLightTickJob(chunk* c, bool causesPriorityUpdate, bool pushJobBack) {

        c->loadStatusMutex().lock();
        if (c->loadStatus() < chunkLoadStatus::PENDING_LIGHTS_APPLIED) {
            c->loadStatus(chunkLoadStatus::PENDING_LIGHTS_APPLIED);
            c->loadStatusMutex().unlock();

            std::unique_lock<std::mutex> lock(lightTickFunctionsMutex_);

            job* aJob = &lightJobs_->get();

            // Set the task.
            std::tuple<chunk*, bool>* data = new std::tuple<chunk*, bool>(c, causesPriorityUpdate);
            aJob->setTask(processLightJob, data, lightJobs_);

            // Send the job to its corresponding queue.
            lightTasks_->submitJob(aJob, pushJobBack);
        
        }
        else
            c->loadStatusMutex().unlock();

    }

    chunk* chunkManager::registerChunk(const ivec3& chunkPos, bool ownNeighborChunks, bool isSimulated) {

        chunk* c = &chunksPool_.get();
        c->makeEmpty(); // TODO. EL MAKEEMPTY HACERLO UN REQUISITO DE T PARA RECYCLING POOLS???
        c->chunkPos(chunkPos);

        {
            std::unique_lock<std::recursive_mutex> lock(chunksMutex_);

            if (isSimulated) {
            
                simulatedChunks_[chunkPos] = c;
                c->poraqui13 = true;
            
            }
            else {
            
                clientChunks_[chunkPos] = c;
                c->poraqui12 = true;

            }
                
            if (ownNeighborChunks) {
            
                c->getAndOwnChunks();
                c->poraqui11 = true;
            
            }
            else
                c->poraqui10 = true;
        }

        return c;

    }

    void chunkManager::manageChunks() {

        {
            // NEXT. LOS JOBS DE NEIGHBOR QUE SE TIENE QUE ACTUALIZAR CON PRIORITY DEBEN IR EN UN MISMO JOB SINO DA PROBLEMAS.

            std::unique_lock<std::mutex> lock(managerThreadMutex_);
            bool continueCreatingChunks = false;
            ivec3 chunkPos = vec3Zero;
            unsigned int nIterations = 0;
            const unsigned int defaultMaxIterations = 128;
            unsigned int maxIterations = defaultMaxIterations;

            // First of all, load the chunk where the player is in.
            if (game::threadsExecute[2]) {

                playerChunkPosCopy_ = camera::cPlayerCamera()->chunkPos();
                ensureChunkIfVisible(playerChunkPosCopy_.x, playerChunkPosCopy_.y, playerChunkPosCopy_.z); // NEXT. ASEGURARSE DE QUE ESTE CHUNK SEA EL DEL JUGADOR YA POSICIONADO BIEN TRAS CARGA DE MUNDO.

            }

            while (game::threadsExecute[2]) {

                continueCreatingChunks = false;
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

                        if (chunkInRenderDistance(chunkPos)) {
                            
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
                        else {

                            frontierIt_++;
                            unloadFrontierChunk(chunkPos);

                            // TODO. HAY QUE AÑADIR UN MIENTRAS HAYA UNA TAREA ASOCIADA A ESTE FRONTIER NO BORRARLO.
                            chunkVBOopsWrite_->operator[](chunkPos) = chunkVBOop(VBOop::FREE); // TODO. NECESARIO???
                            
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
                updateReadChunkMeshes();
                managerThreadCV_.wait(lock);

                {

                    using namespace std::chrono_literals;
                    std::this_thread::sleep_for(1ms);

                }

            }

        }

    }

    void chunkManager::manageChunkPriorityUpdates() {

        std::unique_lock<std::mutex> lock(priorityManagerThreadMutex_);
        std::unique_lock<std::mutex> lockNewChunksMeshes(priorityNewChunkMeshesMutex_);

        while (game::threadsExecute[2]) {

            priorityNewChunkMeshesCV_.wait(lockNewChunksMeshes);

            // Sync with the rendering thread in order to pass it the most updated
            // version of the chunks' meshes.
            updatePriorityReadChunkMeshes();
            priorityManagerThreadCV_.wait(lock);

            {

                using namespace std::chrono_literals;
                std::this_thread::sleep_for(1ms);

            }

        }

    }

    void chunkManager::openedTerrainFileName(const std::string& newFilename) {

        if (game::selectedEngineMode() == VoxelEng::engineMode::EDITLEVEL)
            logger::errorLog("Cannot change the opened terrain file name while in a level");
        else
            openedTerrainFileName_ = newFilename;

    }

    bool chunkManager::ensureChunkIfVisible(const ivec3& chunkPos) {

        chunk* c = getChunk_(chunkPos);
        if (c)
            c->poraqui25 = true;

        if (chunkInRenderDistance(chunkPos)) {

            chunkExistence existence = chunkExistence::NOEXISTS;
            c = getChunk_(chunkPos, existence, true);
            c->poraqui2 = true;
            if (existence == chunkExistence::SIMULATED) {
            
                simulatedChunkToCommon_(c);
                return true;
            
            }
            else if (existence == chunkExistence::COMMON) {

                c->poraqui3 = true;
                c->loadStatusMutex().lock();
                if (c->loadStatus() == chunkLoadStatus::NOTLOADED) {
                    c->loadStatusMutex().unlock();

                    loadOrRemesh_(c);
                
                }
                else
                    c->loadStatusMutex().unlock();
                
                return true;

            }
            else
                return false;

        }
        else
            return false;

    }

    std::string chunkManager::serializeChunk(chunk* c) {

        timer t;
        t.start();

        c->blocksDataMutex().lock_shared();

        // Save local ID data (including duplicated data about neighbors).
        const chunkBlockData cBlockData = c->blockData();
        const Padded3DArray<unsigned short>& blocks = c->blocks();
        std::string data(c->blocks().size() * sizeof(unsigned short), 0);
        std::memcpy(data.data(), c->blocks().data(), c->blocks().size() * sizeof(unsigned short));

        data += '@';

        std::string isOpaqueData(cBlockData.isOpaque_->size() * sizeof(bool), 0);
        std::memcpy(isOpaqueData.data(), cBlockData.isOpaque_->data(), cBlockData.isOpaque_->size() * sizeof(bool));

        data += isOpaqueData;
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

        // Save block lights.
        const std::unordered_set<ivec3>& floodPointLightPositions = c->getFloodPointLightPositions();
        for (auto it = floodPointLightPositions.cbegin(); it != floodPointLightPositions.cend(); it++)
            data += std::to_string((int)it->x) + '|' + std::to_string((int)it->y) + '|' + std::to_string((int)it->z) + '|';
        data += '@';

        // Save block counters
        data += std::to_string(c->nOpaqueBlocks()) + '|' + std::to_string(c->nOpaqueBlocksPlusX()) + '|' + std::to_string(c->nOpaqueBlocksMinusX()) + '|' + std::to_string(c->nOpaqueBlocksPlusY()) + '|' + std::to_string(c->nOpaqueBlocksMinusY()) + '|' + std::to_string(c->nOpaqueBlocksPlusZ()) + '|' + std::to_string(c->nOpaqueBlocksMinusZ()) + '|';
        data += '@';
        data += std::to_string(c->nTotalBlocks()) + '|' + std::to_string(c->nTotalBlocksPlusX()) + '|' + std::to_string(c->nTotalBlocksMinusX()) + '|' + std::to_string(c->nTotalBlocksPlusY()) + '|' + std::to_string(c->nTotalBlocksMinusY()) + '|' + std::to_string(c->nTotalBlocksPlusZ()) + '|' + std::to_string(c->nTotalBlocksMinusZ()) + '|';

        c->blocksDataMutex().unlock_shared();

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

        chunk->blocksDataMutex().lock();

        // Deserialize local block ID data.
        chunkBlockData cBlockData = chunk->blockData();
        Padded3DArray<unsigned short>& blocks = chunk->blocks();
        std::memcpy(blocks.data(), dataBegin, blocks.size() * sizeof(unsigned short));
        index += blocks.size() * sizeof(unsigned short) + 1; // +1 to skip the '@' delimiter character.

        std::memcpy(cBlockData.isOpaque_->data(), dataBegin + index, cBlockData.isOpaque_->size() * sizeof(bool));
        index += cBlockData.isOpaque_->size() * sizeof(bool) + 1; // +1 to skip the '@' delimiter character.

        // Deserialize palette.
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

        // Deserialize block lights.
        std::unordered_set<ivec3>& floodPointLightPositions = chunk->getFloodPointLightPositions();
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
                    throw std::runtime_error("This is not possible. Something has gone horribly wrong when loading chunk from disk");
                }

                word.clear();
                c = data[++index];

            }

            ivec3 pos(x, y, z);
            floodPointLightPositions.insert(pos);

            unsigned short localID = cBlockData.blocksLocalIDs_->at(x,y,z);
            unsigned int globalID = localID ? chunkPalette.getT2(localID) : 0;
            chunk->setBlockLight(block::getBlockC(globalID), pos);

        }

        chunk->blocksDataMutex().unlock();

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

        chunk->needsRemesh(true); // TODO. EL BUG ES QUE SI MODIFICO UN BLOCK EN UN BORDE, TAMBIEN HAY QUE GUARDAR EL CHUNK VECINO QUE LE HACE FRONTERA.

    }

    chunk* chunkManager::loadFrontierChunk(const ivec3& chunkPos) {

        chunk* c = registerChunk(chunkPos, true);

        addFrontier(c);

        c->loadStatusMutex().lock();
        if (c->loadStatus() == chunkLoadStatus::NOTLOADED) {
            c->loadStatus(chunkLoadStatus::AWAITING_LOAD);
            c->loadStatusMutex().unlock();

            // Submit async task to load the chunk either from disk or by generating it.
            issueChunkJob(chunkJobType::LOAD, c); // TODO. PRIORIZAR LOS SIGUIENTES CHUNKS MÁS CERCANOS AL JUGADOR EN LA DIRECCIÓN A LA QUE SE ESTÁ MOVIENDO
            c->poraqui8 = true;
        
        }
        else
        {
            c->loadStatusMutex().unlock();
            c->poraqui5 = true;
        }
           
        return c;

    }

    void chunkManager::issueChunkJob(chunkJobType type, chunk* c, bool pushJobBack) {

        bool send = true;
        job* aJob = &loadChunkJobs_->get();

        // Set the task.
        switch (type) {

        case chunkJobType::NONE:
            logger::errorLog("No chunk job type was specified");
            break;
        case chunkJobType::LOAD:
            aJob->setTask(loadChunkJob, c, loadChunkJobs_);
            break;
        case chunkJobType::LOAD2:
            c->loadStatusMutex().lock();
            if (c->loadStatus() == chunkLoadStatus::BASICTERRAIN || c->loadStatus() == chunkLoadStatus::BASICTERRAINFROMDISK) {
                c->loadStatus(chunkLoadStatus::PENDING_DECORATED);
                aJob->setTask(loadChunkJobPass2, c, loadChunkJobs_);
            }
            else
                send = false;
            c->loadStatusMutex().unlock();
            break;
        case chunkJobType::ONLYREMESH:
            aJob->setTask(remeshChunkJob, c, loadChunkJobs_);
            break;
        case chunkJobType::UNLOADANDSAVE:
            aJob->setTask(unloadAndSaveChunkJob, c, loadChunkJobs_);
            break;
        case chunkJobType::PRIORITYREMESH:
            aJob->setTask(priorityRemeshChunkJob, c, loadChunkJobs_);
            break;
        default:
            logger::errorLog("Unsupported chunkJobType type " + std::to_string(static_cast<int>(type)));
            break;
        }

        // Send the job to its corresponding queue.
        if (send) {

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
        else
            loadChunkJobs_->free(*aJob);
        
    }

    void chunkManager::clear() {

        if (chunkTasks_)
            chunkTasks_->awaitNoJobs();

        if (priorityChunkTasks_)
            priorityChunkTasks_->awaitNoJobs();

        {

            std::unique_lock<std::mutex> lock(lightTickFunctionsMutex_);

            if (lightTasks_) {

                lightTasks_->shutdown();
                lightTasks_->awaitTermination();

                lightJobs_->clear();
                pendingLightJobs_.clear();

                delete lightTasks_;
                lightTasks_ = nullptr;

            }

        }

        {
            std::unique_lock<std::recursive_mutex> lock(chunksMutex_);
            for (auto it = clientChunks_.begin(); it != clientChunks_.end(); it++)
                if (it->second)
                    delete it->second;
            clientChunks_.clear();

            for (auto it = simulatedChunks_.begin(); it != simulatedChunks_.end(); it++)
                if (it->second)
                    delete it->second;
            simulatedChunks_.clear();
        }
        
        if (chunkMeshes_)
            chunkMeshes_->clear();

        if (chunkVBOopsRead_)
            chunkVBOopsRead_->clear();

        if (chunkVBOopsPriorityRead_)
            chunkVBOopsPriorityRead_->clear();

        if (chunkVBOopsWrite_)
            chunkVBOopsWrite_->clear();

        if (chunkVBOopsPriorityWrite_)
            chunkVBOopsPriorityWrite_->clear();

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

        chunkNeighborsInfoMutex_.lock();
        chunkNeighborsInfo_.clear();
        chunkNeighborsInfoMutex_.unlock();

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

        if (chunkMeshes_)
            chunkMeshes_->clear();

        if (chunkVBOopsRead_)
            chunkVBOopsRead_->clear();

        if (chunkVBOopsPriorityRead_)
            chunkVBOopsPriorityRead_->clear();

        if (chunkVBOopsWrite_)
            chunkVBOopsWrite_->clear();

        if (chunkVBOopsPriorityWrite_)
            chunkVBOopsPriorityWrite_->clear();

        initialised_ = false;

    }

    chunk* chunkManager::getChunk_(const ivec3& chunkPos, chunkExistence& existence, bool createIfDoesntExist) {

        chunksMap::const_iterator it;
        chunksMap::const_iterator it2;
        bool foundClientChunk = false;
        bool foundSimulatedChunk = false;
        chunk* c = nullptr;
        {
            std::unique_lock<std::recursive_mutex> lock(chunksMutex_);
            it = clientChunks_.find(chunkPos);
            it2 = simulatedChunks_.find(chunkPos);
            foundClientChunk = it != clientChunks_.cend();
            foundSimulatedChunk = it2 != simulatedChunks_.cend();
            if (foundClientChunk)
                c = it->second;
            else if (foundSimulatedChunk)
                c = it2->second;
        }

        if (foundClientChunk) {

            c->poraqui4 = true;

            existence = chunkExistence::COMMON;
            return c;

        }
        else if (foundSimulatedChunk) {

            c->poraqui6 = true;

            existence = chunkExistence::SIMULATED;
            return c;

        }
        else {

            existence = chunkExistence::NOEXISTS;

            if (createIfDoesntExist) {

                existence = chunkExistence::COMMON;
                c = loadFrontierChunk(chunkPos);
                c->poraqui7 = true;
                return c;

            }
            else {

                existence = chunkExistence::NOEXISTS;
                return nullptr;

            }

        }

    }

    const block& chunkManager::getBlockOGWorld_(int posX, int posY, int posZ) {

        ivec3 chunkPos = getChunkCoords(posX, posY, posZ);
        std::unique_lock<std::recursive_mutex> lock(chunksMutex_);
        chunksMap::const_iterator it = clientChunks_.find(chunkPos);
        if (it == clientChunks_.end())
            logger::errorLog("Chunk " + std::to_string(chunkPos) + " does not exist");
        else {
        
            chunk* c = it->second;
            return c->get<blockProperty>(floorMod(posX, CHUNK_SIZE), floorMod(posY, CHUNK_SIZE), floorMod(posZ, CHUNK_SIZE)).b;
        
        }
            
    }

    void chunkManager::pushNewChunkMesh_(bool isPriorityUpdate, chunk* c, std::size_t meshSize) {

        c->poraqui20 = true;

        if (isPriorityUpdate) {

            c->poraqui21 = true;
            priorityNewChunkMeshesMutex_.lock();
            priorityNewChunkMeshes_.push_back(c);
            priorityNewChunkMeshesMutex_.unlock();

            priorityNewChunkMeshesCV_.notify_all();

        }
        else if (meshSize) {

            c->poraqui22 = true;
            newChunkMeshesMutex_.lock();
            newChunkMeshes_.push_back(c);
            newChunkMeshesMutex_.unlock();

        }

    }

    void chunkManager::simulatedChunkToCommon_(chunk* c) {

        const ivec3& chunkPos = c->chunkPos();

        addFrontier(c);

        {
            std::unique_lock<std::recursive_mutex> lock(chunksMutex_);
            clientChunks_[chunkPos] = c;
            simulatedChunks_.erase(chunkPos);
            c->unloadWhenNoOwners(false);
        }

        loadOrRemesh_(c);

    }

    void chunkManager::loadOrRemesh_(chunk* c) {
    
        c->loadStatusMutex().lock();
        if (c->loadStatus() == chunkLoadStatus::NOTLOADED) {
            c->loadStatus(chunkLoadStatus::AWAITING_LOAD);
            c->loadStatusMutex().unlock();

            c->getAndOwnChunks();
            issueChunkJob(chunkJobType::LOAD, c);
            c->poraqui = true;

        }
        else {
            c->loadStatusMutex().unlock();

            c->poraqui9 = true;

            increaseNeighborInfoPassCounter_(c);
            if (c->needsRemesh())
                issueChunkJob(chunkJobType::ONLYREMESH, c);

        }
    
    }

    void chunkManager::increaseNeighborInfoPassCounter_(chunk* c, bool sendLoad2JobWhenRequired) {

        const ivec3& chunkPos = c->chunkPos();

        std::shared_ptr<neighborsInfo> info = getOrCreateChunkNeighborInfo(chunkPos);

        info->neighborsGenPass1Completed_.lock();
        std::set<ivec3>& completed = info->neighborsGenPass1Completed_.get();
        completed.insert(chunkPos);
        if (sendLoad2JobWhenRequired && completed.size() == 27)
            issueChunkJob(chunkJobType::LOAD2, c);
        info->neighborsGenPass1Completed_.unlock();

        ivec3 neighborPos;
        for (const ivec3& offset : neighborsOffsets) {

            neighborPos = chunkPos + offset;

            std::shared_ptr<neighborsInfo> infoNeighbor = getOrCreateChunkNeighborInfo(neighborPos);

            chunk* neighbor = getChunk_(neighborPos);

            infoNeighbor->neighborsGenPass1Completed_.lock();
            std::set<ivec3>& neighborCompleted = infoNeighbor->neighborsGenPass1Completed_.get();
            neighborCompleted.insert(chunkPos);
            if (sendLoad2JobWhenRequired && neighbor && neighborCompleted.size() == 27)
                issueChunkJob(chunkJobType::LOAD2, neighbor);
            if(neighbor)
                neighbor->poraqui28 = true;
            infoNeighbor->neighborsGenPass1Completed_.unlock();

        }

        c->poraqui27 = true;

    }

    void chunkManager::decreaseNeighborInfoPassCounter_(chunk& c) {

        // Remove neighborInfo data originating from this chunk.
        const ivec3& chunkPos = c.chunkPos();
        chunkNeighborsInfoMutex_.lock();
        auto info = chunkNeighborsInfo_.find(chunkPos);
        bool thereIsInfo = info != chunkNeighborsInfo_.cend();
        chunkNeighborsInfoMutex_.unlock();

        if (thereIsInfo) {

            std::shared_ptr<neighborsInfo> n = info->second;
            n->neighborsGenPass1Completed_.lock();
            std::set<ivec3>& completed = n->neighborsGenPass1Completed_.get();
            completed.erase(chunkPos);
            if (completed.empty()) {

                n->neighborsGenPass1Completed_.unlock();

                chunkNeighborsInfoMutex_.lock();
                chunkNeighborsInfo_.erase(chunkPos); // TODO. REUSABLE NEIGHBORINFO OBJECTS???
                chunkNeighborsInfoMutex_.unlock();

            }
            else
                n->neighborsGenPass1Completed_.unlock();

            ivec3 neighborPos;
            for (const ivec3& offset : neighborsOffsets) {

                neighborPos = chunkPos + offset;

                chunkNeighborsInfoMutex_.lock();
                info = chunkNeighborsInfo_.find(neighborPos);
                thereIsInfo = info != chunkNeighborsInfo_.cend();
                chunkNeighborsInfoMutex_.unlock();

                if (thereIsInfo)
                {
                    std::shared_ptr<neighborsInfo> neighborInfo = info->second;
                    neighborInfo->neighborsGenPass1Completed_.lock();
                    std::set<ivec3>& neighborCompleted = neighborInfo->neighborsGenPass1Completed_.get();
                    neighborCompleted.erase(neighborPos);
                    if (neighborCompleted.empty()) {

                        neighborInfo->neighborsGenPass1Completed_.unlock();

                        chunkNeighborsInfoMutex_.lock();
                        chunkNeighborsInfo_.erase(chunkPos); // TODO. REUSABLE NEIGHBORINFO OBJECTS???
                        chunkNeighborsInfoMutex_.unlock();

                    }
                    else
                        neighborInfo->neighborsGenPass1Completed_.unlock();

                }

            }

        }

    }

    void chunkManager::processWorldLightUpdates_() {

        try {

            while (game::threadsExecute[1]) {

                std::unique_lock<std::mutex> lock(lightTickFunctionsMutex_);
                auto it = pendingLightJobs_.begin();
                while (!pendingLightJobs_.empty()) {

                    (*it)(); // Execute the job.
                    it = pendingLightJobs_.erase(it);

                }

                {

                    using namespace std::chrono_literals;
                    std::this_thread::sleep_for(1ms); // TODO. PONER AQUÍ QUE SE DESCANSE UN TICK DE MAINCRA (0.05s) REDUCIENDO EL TIEMPO GASTADO EN PROCESAR ESTE TICK

                }

            }

        }
        catch (...) {

            VoxelEng::logger::say("Error was detected during engine execution. Shutting down light tick functions management thread.");
            game::setLoopSelection(engineMode::EXIT);

        }

    }

    void chunkManager::recalculateBlockLight_(chunk& c, bool priorityUpdate) {

        std::unique_lock<std::recursive_mutex> lock(c.ownedChunksMutex());
        const std::unordered_map<ivec3, chunk*>& ownedChunks = c.ownedChunks();

        if (!ownedChunks.empty()) {
        
            std::unordered_map<ivec3, chunkBlockData> ownedBlockData; // Chunk pos offset is used as key.
            std::unordered_map<ivec3, Padded3DArray<char>> blockLightIntensity; // TODO. COMO SOLO HAY 26 VECINOS, CAMBIAR UNORDERED MAP POR UNA ESTRUCURA QUE HAGA UN SWITCH.
            bool skipJob = false;
            {
                const ivec3& chunkPos = c.chunkPos();
                chunk* neighbor = nullptr;
                std::unique_lock<std::recursive_mutex> lock(chunksMutex_);

                ownedBlockData[vec3Zero] = c.blockData();
                blockLightIntensity.emplace(std::piecewise_construct,
                    std::forward_as_tuple(vec3Zero),
                    std::forward_as_tuple(CHUNK_SIZE, CHUNK_SIZE, CHUNK_SIZE, 1, 0));
                for (auto it = neighborsOffsets.cbegin(); !skipJob && it != neighborsOffsets.cend(); it++) {

                    const ivec3& chunkPosOffset = *it;
                    if (neighbor = ownedChunks.at(chunkPosOffset)) {

                        ownedBlockData[chunkPosOffset] = neighbor->blockData();
                        blockLightIntensity.emplace(std::piecewise_construct,
                            std::forward_as_tuple(chunkPosOffset),
                            std::forward_as_tuple(CHUNK_SIZE, CHUNK_SIZE, CHUNK_SIZE, 1, 0));

                    }
                    else
                        skipJob = true;

                }

            }

            // Propagate every light across the chunk and its neighbors without taking into account chunk borders.
            if (!skipJob) {

                const std::unordered_set<ivec3>& floodPointLightPositions = c.getFloodPointLightPositions();
                ivec3 chunkPosOffset = vec3Zero;
                ivec3 chunkRelPos = vec3Zero;
                ivec3 neighborOffset = vec3Zero;
                ivec3 neighborPos = vec3Zero;
                ivec3 neighborRelPos = vec3Zero;
                std::deque<blockLightMod> floodLightsInstances;
                bool firstSecondLoopIteration = false;
                char neighborIntensity = 0; // TODO. cambiar por char& ???
                for (auto it = floodPointLightPositions.cbegin(); it != floodPointLightPositions.cend(); it++) {

                    // Reset first loop variables.
                    for (auto it = neighborsOffsets.cbegin(); !skipJob && it != neighborsOffsets.cend(); it++)
                        blockLightIntensity[*it].clear();
                    floodLightsInstances.clear();

                    // Add block's light and init second loop variables.
                    const ivec3* blockGlobalPos = &*it; // We assume the chunk is the center of the coordinate system.
                    chunkPosOffset = getChunkCoords(*blockGlobalPos);
                    chunkRelPos = getChunkRelCoords(*blockGlobalPos);
                    const chunkBlockData* blockData = &ownedBlockData[chunkPosOffset]; // TODO. AÑADIR FUNCION SWITCH PARA NO USAR EL HASH DE DICCIONARIO
                    floodLightsInstances.emplace_back(*blockGlobalPos,
                        blockData->blockLightLevel_->at(chunkRelPos), blockData->blockLightColor_->at(chunkRelPos));
                    if (floodLightsInstances.size() > 0) {

                        firstSecondLoopIteration = true;
                        blockLightMod* floodLight = &floodLightsInstances.front();
                        do {

                            // MAÑANA. NO SE ESTÁ RECALCULANDO EL IS OPAQUE CUANDO SE DESERIALIZA UN CHUNK GUARDADO
                            if ((firstSecondLoopIteration || !blockData->isOpaque_->at(chunkRelPos)) &&
                                floodLight->intensity > blockLightIntensity[chunkPosOffset][chunkRelPos.x][chunkRelPos.y][chunkRelPos.z]) {

                                // Apply light on position.
                                chunk* ownedChunk = ownedChunks.at(chunkPosOffset);
                                ownedChunk->addBlockLight(chunkRelPos, floodLight->color, floodLight->intensity, true);
                                blockLightIntensity[chunkPosOffset][chunkRelPos.x][chunkRelPos.y][chunkRelPos.z] = floodLight->intensity;

                                // Calculate light blending across chunks.
                                neighborOffset.x = chunkRelPos.x >= CHUNK_SIZE_LIMIT ? 1 : chunkRelPos.x <= 0 ? -1 : 0;
                                neighborOffset.y = chunkRelPos.y >= CHUNK_SIZE_LIMIT ? 1 : chunkRelPos.y <= 0 ? -1 : 0;
                                neighborOffset.z = chunkRelPos.z >= CHUNK_SIZE_LIMIT ? 1 : chunkRelPos.z <= 0 ? -1 : 0;
                                if (neighborOffset.x != 0) {

                                    if (neighborOffset.y != 0) {

                                        // X Y Z
                                        if (neighborOffset.z != 0) {

                                            neighborPos = chunkPosOffset + neighborOffset;
                                            neighborRelPos = getChunkRelCoords(chunkRelPos + neighborOffset);
                                            neighborIntensity = blockLightIntensity[neighborPos][neighborRelPos];

                                            if (floodLight->intensity - 1 > neighborIntensity) {

                                                // Apply light to both neighbors so that smooth lighting's blending can be made correctly.
                                                ownedChunk->addBlockLight(chunkRelPos + neighborOffset,
                                                    floodLight->color, floodLight->intensity - 1, true);
                                                blockLightIntensity[chunkPosOffset][chunkRelPos + neighborOffset]
                                                    = floodLight->intensity - 1;

                                                ownedChunks.at(neighborPos)->addBlockLight(neighborRelPos - neighborOffset,
                                                    floodLight->color, floodLight->intensity, true);
                                                blockLightIntensity[neighborPos][neighborRelPos - neighborOffset]
                                                    = floodLight->intensity;

                                            }

                                        }

                                        // X Y 0
                                        neighborPos = ivec3(chunkPosOffset.x + neighborOffset.x, chunkPosOffset.y + neighborOffset.y, chunkPosOffset.z);
                                        neighborRelPos =
                                            getChunkRelCoords(ivec3(chunkRelPos.x + neighborOffset.x, chunkRelPos.y + neighborOffset.y, chunkRelPos.z));
                                        neighborIntensity = blockLightIntensity[neighborPos][neighborRelPos];

                                        if (floodLight->intensity - 1 > neighborIntensity) {

                                            // Apply light to both neighbors so that smooth lighting's blending can be made correctly.
                                            ownedChunk->addBlockLight(chunkRelPos, neighborOffset.x, neighborOffset.y, 0,
                                                floodLight->color, floodLight->intensity - 1, true);
                                            blockLightIntensity[chunkPosOffset][chunkRelPos.x + neighborOffset.x][chunkRelPos.y + neighborOffset.y][chunkRelPos.z]
                                                = floodLight->intensity - 1;

                                            ownedChunks.at(neighborPos)->addBlockLight(neighborRelPos, -neighborOffset.x, -neighborOffset.y, 0,
                                                floodLight->color, floodLight->intensity, true);
                                            blockLightIntensity[neighborPos][chunkRelPos.x - neighborOffset.x][chunkRelPos.y - neighborOffset.y][chunkRelPos.z]
                                                = floodLight->intensity;

                                        }

                                    }

                                    // X 0 Z
                                    if (neighborOffset.z != 0) {

                                        neighborPos = ivec3(chunkPosOffset.x + neighborOffset.x, chunkPosOffset.y, chunkPosOffset.z + neighborOffset.z);
                                        neighborRelPos =
                                            getChunkRelCoords(ivec3(chunkRelPos.x + neighborOffset.x, chunkRelPos.y, chunkRelPos.z + neighborOffset.z));
                                        neighborIntensity = blockLightIntensity[neighborPos][neighborRelPos];

                                        if (floodLight->intensity - 1 > neighborIntensity) {

                                            // Apply light to both neighbors so that smooth lighting's blending can be made correctly.
                                            ownedChunk->addBlockLight(chunkRelPos, neighborOffset.x, 0, neighborOffset.z,
                                                floodLight->color, floodLight->intensity - 1, true);
                                            blockLightIntensity[chunkPosOffset][chunkRelPos.x + neighborOffset.x][chunkRelPos.y][chunkRelPos.z + neighborOffset.z]
                                                = floodLight->intensity - 1;

                                            ownedChunks.at(neighborPos)->addBlockLight(neighborRelPos, -neighborOffset.x, 0, -neighborOffset.z,
                                                floodLight->color, floodLight->intensity, true);
                                            blockLightIntensity[neighborPos][chunkRelPos.x - neighborOffset.x][chunkRelPos.y][chunkRelPos.z - neighborOffset.z]
                                                = floodLight->intensity;

                                        }

                                    }

                                    // X 0 0
                                    neighborPos = ivec3(chunkPosOffset.x + neighborOffset.x, chunkPosOffset.y, chunkPosOffset.z);
                                    neighborRelPos = getChunkRelCoords(ivec3(chunkRelPos.x + neighborOffset.x, chunkRelPos.y, chunkRelPos.z));
                                    neighborIntensity = blockLightIntensity[neighborPos][neighborRelPos];

                                    if (floodLight->intensity - 1 > neighborIntensity) {

                                        // Apply light to both neighbors so that smooth lighting's blending can be made correctly.
                                        ownedChunk->addBlockLight(chunkRelPos, neighborOffset.x, 0, 0,
                                            floodLight->color, floodLight->intensity - 1, true);
                                        blockLightIntensity[chunkPosOffset][chunkRelPos.x + neighborOffset.x][chunkRelPos.y][chunkRelPos.z]
                                            = floodLight->intensity - 1;

                                        ownedChunks.at(neighborPos)->addBlockLight(neighborRelPos, -neighborOffset.x, 0, 0,
                                            floodLight->color, floodLight->intensity, true);
                                        blockLightIntensity[neighborPos][chunkRelPos.x - neighborOffset.x][chunkRelPos.y][chunkRelPos.z]
                                            = floodLight->intensity;

                                    }

                                }

                                if (neighborOffset.y != 0) {

                                    // 0 Y Z
                                    if (neighborOffset.z != 0) {

                                        neighborPos = ivec3(chunkPosOffset.x, chunkPosOffset.y + neighborOffset.y, chunkPosOffset.z + neighborOffset.z);
                                        neighborRelPos = getChunkRelCoords(ivec3(chunkRelPos.x, chunkRelPos.y + neighborOffset.y, chunkRelPos.z + neighborOffset.z));
                                        neighborIntensity = blockLightIntensity[neighborPos][neighborRelPos];

                                        if (floodLight->intensity - 1 > neighborIntensity) {

                                            // Apply light to both neighbors so that smooth lighting's blending can be made correctly.
                                            ownedChunk->addBlockLight(chunkRelPos, 0, neighborOffset.y, neighborOffset.z,
                                                floodLight->color, floodLight->intensity - 1, true);
                                            blockLightIntensity[chunkPosOffset][chunkRelPos.x][chunkRelPos.y + neighborOffset.y][chunkRelPos.z + neighborOffset.z]
                                                = floodLight->intensity - 1;

                                            ownedChunks.at(neighborPos)->addBlockLight(neighborRelPos, 0, -neighborOffset.y, -neighborOffset.z,
                                                floodLight->color, floodLight->intensity, true);
                                            blockLightIntensity[neighborPos][chunkRelPos.x][chunkRelPos.y - neighborOffset.y][chunkRelPos.z - neighborOffset.z]
                                                = floodLight->intensity;

                                        }
                                    }

                                    // 0 Y 0
                                    neighborPos = ivec3(chunkPosOffset.x, chunkPosOffset.y + neighborOffset.y, chunkPosOffset.z);
                                    neighborRelPos = getChunkRelCoords(ivec3(chunkRelPos.x, chunkRelPos.y + neighborOffset.y, chunkRelPos.z));
                                    neighborIntensity = blockLightIntensity[neighborPos][neighborRelPos];

                                    if (floodLight->intensity - 1 > neighborIntensity) {

                                        // Apply light to both neighbors so that smooth lighting's blending can be made correctly.
                                        ownedChunk->addBlockLight(chunkRelPos, 0, neighborOffset.y, 0,
                                            floodLight->color, floodLight->intensity - 1, true);
                                        blockLightIntensity[chunkPosOffset][chunkRelPos.x][chunkRelPos.y + neighborOffset.y][chunkRelPos.z]
                                            = floodLight->intensity - 1;

                                        ownedChunks.at(neighborPos)->addBlockLight(neighborRelPos, 0, -neighborOffset.y, 0,
                                            floodLight->color, floodLight->intensity, true);
                                        blockLightIntensity[neighborPos][chunkRelPos.x][chunkRelPos.y - neighborOffset.y][chunkRelPos.z]
                                            = floodLight->intensity;

                                    }

                                }

                                // 0 0 Z
                                if (neighborOffset.z != 0) {

                                    neighborPos = ivec3(chunkPosOffset.x, chunkPosOffset.y, chunkPosOffset.z + neighborOffset.z);
                                    neighborRelPos = getChunkRelCoords(ivec3(chunkRelPos.x, chunkRelPos.y, chunkRelPos.z + neighborOffset.z));
                                    neighborIntensity = blockLightIntensity[neighborPos][neighborRelPos];

                                    if (floodLight->intensity - 1 > neighborIntensity) {

                                        // Apply light to both neighbors so that smooth lighting's blending can be made correctly.
                                        ownedChunk->addBlockLight(chunkRelPos, 0, 0, neighborOffset.z,
                                            floodLight->color, floodLight->intensity - 1, true);
                                        blockLightIntensity[chunkPosOffset][chunkRelPos.x][chunkRelPos.y][chunkRelPos.z + neighborOffset.z]
                                            = floodLight->intensity - 1;

                                        // TODO. UNA VEZ SEPAMOS QUE ESTO ES SEGURO CAMBIAR AT POR OPERATOR[]
                                        ownedChunks.at(neighborPos)->addBlockLight(neighborRelPos, 0, 0, -neighborOffset.z,
                                            floodLight->color, floodLight->intensity, true);
                                        blockLightIntensity[neighborPos][chunkRelPos.x][chunkRelPos.y][chunkRelPos.z - neighborOffset.z]
                                            = floodLight->intensity;

                                    }

                                }

                                // Spread light.

                                //+x
                                floodLightsInstances.emplace_back(*blockGlobalPos + ivec3FixedNorth, floodLight->intensity - 1, floodLight->color);

                                //-x
                                floodLightsInstances.emplace_back(*blockGlobalPos + ivec3FixedSouth, floodLight->intensity - 1, floodLight->color);

                                //+y
                                floodLightsInstances.emplace_back(*blockGlobalPos + ivec3FixedUp, floodLight->intensity - 1, floodLight->color);

                                //-y
                                floodLightsInstances.emplace_back(*blockGlobalPos + ivec3FixedDown, floodLight->intensity - 1, floodLight->color);

                                //+z
                                floodLightsInstances.emplace_back(*blockGlobalPos + ivec3FixedEast, floodLight->intensity - 1, floodLight->color);

                                //-z
                                floodLightsInstances.emplace_back(*blockGlobalPos + ivec3FixedWest, floodLight->intensity - 1, floodLight->color);

                            }
                            floodLightsInstances.pop_front();

                            if (floodLightsInstances.size() > 0) {

                                // Reset loop variables
                                firstSecondLoopIteration = false;
                                floodLight = &floodLightsInstances.front();
                                blockGlobalPos = &floodLight->pos; // We assume the chunk is the center of the coordinate system.
                                chunkPosOffset = getChunkCoords(*blockGlobalPos);
                                chunkRelPos = getChunkRelCoords(*blockGlobalPos);
                                blockData = &ownedBlockData[chunkPosOffset]; // TODO. AÑADIR FUNCION SWITCH PARA NO USAR EL HASH DE DICCIONARIO

                            }

                        } while (floodLightsInstances.size() > 0);

                    }

                }

                // Remesh chunks.
                for (auto it = ownedChunks.cbegin(); it != ownedChunks.cend(); it++) {

                    it->second->loadStatusMutex().lock();
                    if (it->second->needsRemesh() && it->second->loadStatus() >= chunkLoadStatus::MESHED) {

                        it->second->loadStatusMutex().unlock();
                        remesh(it->second, priorityUpdate);

                    }
                    else
                        it->second->loadStatusMutex().unlock();

                }

            }
        
        }

    }

    void chunkManager::loadChunkJob(void* data) {

        chunk* c = static_cast<chunk*>(data);
        if (world::isSaved(c->chunkPos())) {  // Load previously saved chunk. Skips remaining loading passes.

            deserializeChunk(c, world::loadChunk(c->chunkPos()));
            {
                std::unique_lock<std::recursive_mutex> lock(c->loadStatusMutex());
                c->loadStatus(chunkLoadStatus::BASICTERRAINFROMDISK);
            }

        }
        else { // Generate new chunk.

            worldGen::generate(*c);
            {
                std::unique_lock<std::recursive_mutex> lock(c->loadStatusMutex());
                c->loadStatus(chunkLoadStatus::BASICTERRAIN);
            }

        }
        increaseNeighborInfoPassCounter_(c, true);

    }

    void chunkManager::loadChunkJobPass2(void* data) {

        chunk* c = static_cast<chunk*>(data);

        worldGen::genPass2(*c);

        c->loadStatusMutex().lock();
        c->loadStatus(chunkLoadStatus::DECORATED);
        c->loadStatusMutex().unlock();

        c->postGenPass();

        if (c->getFloodPointLightPositions().empty()) {

            c->poraqui16 = true;
            remesh(c, false);
            c->disownChunks();

        }
        else
            addLightTickJob(c, false, true);

    }

    void chunkManager::remeshChunkJob(void* data) {

        chunk* c = static_cast<chunk*>(data);
        remesh(c, false);

    }

    void chunkManager::unloadAndSaveChunkJob(void* data) {

        chunk* c = static_cast<chunk*>(data);
        if (c->modified()) {

            world::saveChunk(c);
            c->modified(false);
            c->disownChunks();

        }
        c->poraqui24 = true;
        chunksPool_.free(*c);

    }

    void chunkManager::priorityRemeshChunkJob(void* data) {

        chunk* c = static_cast<chunk*>(data);
        c->needsRemesh(true);
        c->getAndOwnChunks();
        recalculateBlockLight_(*c, true);
        c->disownChunks();

    }

    void chunkManager::processLightJob(void* rawData) {

        std::tuple<chunk*, bool>* data = static_cast<std::tuple<chunk*, bool>*>(rawData);
        chunk* c = std::get<chunk*>(*data);
        bool causesPriorityUpdate = std::get<bool>(*data);

        c->loadStatusMutex().lock();
        if (c->loadStatus() == chunkLoadStatus::PENDING_LIGHTS_APPLIED) {
            c->loadStatusMutex().unlock();
        
            recalculateBlockLight_(*c, causesPriorityUpdate);

            delete data; // Does not remove the chunk cause we passed a pointer no a reference.

            c->loadStatusMutex().lock();
            c->loadStatus(chunkLoadStatus::LIGHTS_APPLIED);
            c->loadStatusMutex().unlock();

            c->disownChunks();
        
        }
        else
            c->loadStatusMutex().unlock();

    }

}
