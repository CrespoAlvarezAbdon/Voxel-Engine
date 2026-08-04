#include "chunk.h"

// MAÑANA. PROBAR TODOS LOS CASOS DE LAS LUCES 1 A 1
// - LA MEJORA DE MEMORIA ES DE 3GB ASÍ QUE EL LAZY INITIALIZATION DE LUCES SE QUEDA. <- OK
// - HAY UN BUG QUE HACE QUE SI PONES UN BLOQUE DE LUZ NO SE ACTUALIZA EL CHUNK HASTA QUE AÑADES UN BLOQUE QUE NO DE LUZ. <- ARREGLADO
// - HAY UN BUG EN EL QUE SI PONES UNA LUZ Y LUEGO INTENTAS PONER OTRA NO SE RENDERIZAN ALGUNAS CARAS Y LA LUZ DEL NUEVO BLOQUE NO SE PROPAGA <- ARREGLADO
// - SEGUIR PROBANDO TODOS LOS CASOS DE LAS LUCES DE 1 EN 1
// - ARREGLAR BUG EN EL QUE SI TAPAS UN BLOQUE DE LUZ CON OTROS BLOQUES DE LUZ Y LO DESTAPAS LA LUZ DE ESE BLOQUE NO VUELVE A PROPAGARSE. <- ARREGLADO
// - EL CASO NONTRANSPARENTBLOCKREMOVEDONLIGHT HACE OVERRIDE DE LUZ ROJA AL LIBERAR LUZ AZUL DE CARCEL DE LUCES ROJAS <- ARREGLADO
// - GHOSTING DE LUCES AL HACER LO DE ENCERRAR ROJA EN AZULES DOS VECES (SE NOTA LA PRIMERA VEZ TAMBIÉN COMO QUE PARECE QUE NO SE PROPAGA EL CAMBIO A CHUNKS PUEDE QUE SE DEBA A LO QUE HICIMOS PARA ARREGLAR EL ANTERIOR BUG)

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
#include <Chunk/Utilities/utilities.hpp>
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
        : modified_(false),
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
        : modified_(false),
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
        : blockData_(c.blockData_),
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

        unsigned int localID = blockData_.blocksLocalIDs.get(x, y, z);
        return blockState(localID ? block::getBlockC(palette_.getT2(localID)) : block::emptyBlock(), blockProperty::emptyProp());

    }

    const block& chunk::getNeighborBlock(GLbyte firstIndex, GLbyte secondIndex, blockViewDir neighbor) {

        unsigned int localID = 0;

        switch (neighbor) {
            
        case blockViewDir::PLUSX:
            localID = blockData_.blocksLocalIDs.get(CHUNK_SIZE, firstIndex, secondIndex);
            break;

        case blockViewDir::NEGX:
            localID = blockData_.blocksLocalIDs.get(-1, firstIndex, secondIndex);
            break;

        case blockViewDir::PLUSY:
            localID = blockData_.blocksLocalIDs.get(firstIndex, CHUNK_SIZE, secondIndex);
            break;

        case blockViewDir::NEGY:
            localID = blockData_.blocksLocalIDs.get(firstIndex, -1, secondIndex);
            break;

        case blockViewDir::PLUSZ:
            localID = blockData_.blocksLocalIDs.get(firstIndex, secondIndex, CHUNK_SIZE);
            break;

        case blockViewDir::NEGZ:
            localID = blockData_.blocksLocalIDs.get(firstIndex, secondIndex, -1);
            break;

        }

        return localID ? block::getBlockC(palette_.getT2(localID)) : block::emptyBlock();

    }

    const block& chunk::setBlock(sbyte x, sbyte y, sbyte z, const block& b, bool modification) {

        bool isNewBlockOpaque = b.opacity() == blockOpacity::OPAQUEBLOCK;
        unsigned short& actualLocalID = blockData_.blocksLocalIDs.get(x, y, z);
        unsigned short oldLocalID = actualLocalID;
        unsigned int oldGlobalID = actualLocalID ? palette_.getT2(actualLocalID) : 0;
        const block& oldB = block::getBlockC(oldGlobalID);
        ivec3 neighborOffset;

        const bool blockWasModified = placeNewBlock_(actualLocalID, b);

        needsRemesh_ = needsRemesh_ || blockWasModified;
        modified_ = modified_ || (blockWasModified && modification);

        blockData_.isOpaque.get(x, y, z) = isNewBlockOpaque;

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
            blockData_.floodPointLightPositions.erase(pos);

        addNewBlockLight(b, pos);

    }

    void chunk::addNewBlockLight(const block& b, const ivec3& pos) {

        const varRef& emittedLight = b.emittedLight();
        if (!emittedLight.isNull()) { // Add new light pos if any.

            const pointLight* pointL = emittedLight.pointer<pointLight>();

            blockData_.floodPointLightPositions.insert(pos);

            setBlockLight(pos, blockLight(pointL->intensity(), pointL->ambient()), colorChannel::ALL);

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
            int x = 0,
                y = 0,
                z = 0;
            unsigned short localID = 0;
            unsigned short neighborLocalID = 0;
            vertex aux;
            const block* bNeighbor = nullptr;
            const blockLight& blockLightZero = blockLight::zero();
            if (nTotalBlocks_ && nOpaqueBlocks_ < nBlocksChunk)
                for (x = 0; x < CHUNK_SIZE; x++)
                    for (y = 0; y < CHUNK_SIZE; y++)
                        for (z = 0; z < CHUNK_SIZE; z++) {

                            localID = blockData_.blocksLocalIDs.get(x, y, z);
                            const blockLight& localLight = blockData_.blockLightColor ? blockData_.blockLightColor->get(x, y, z) : blockLightZero;
                            block& b = localID ? block::getBlockC(palette_.getT2(localID)) : block::emptyBlock();
                            const varRef& emittedLight = b.emittedLight();

                            // Add block's model to the mesh if necessary.
                            if (b.opacity() <= blockOpacity::TRANSLUCENTBLOCK) {

                                // Draw face for block at x + 1.
                                if (x < CHUNK_SIZE_LIMIT && (neighborLocalID = blockData_.blocksLocalIDs.get(x + 1, y, z))) {

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
                                                aux.additionalData = getBlockLightAverage_(localLight,
                                                    basicVec3(x, y, z + 1), basicVec3(x, y - 1, z), basicVec3(x, y - 1, z + 1));
                                                aux.lightExtraData.x = 0;
                                                aux.lightExtraData.y = 0;
                                                break;
                                            case 1: // block vertex 7 (C)
                                            case 4:
                                                aux.additionalData = getBlockLightAverage_(localLight,
                                                    basicVec3(x, y, z + 1), basicVec3(x, y + 1, z), basicVec3(x, y + 1, z + 1));
                                                aux.lightExtraData.x = 0;
                                                aux.lightExtraData.y = 1;
                                                break;
                                            case 2: // block vertex 0 (A)
                                            case 3:
                                                aux.additionalData = getBlockLightAverage_(localLight,
                                                    basicVec3(x, y, z - 1), basicVec3(x, y - 1, z), basicVec3(x, y - 1, z - 1));
                                                aux.lightExtraData.x = 1;
                                                aux.lightExtraData.y = 0;
                                                break;
                                            case 5: // block vertex 3 (D)
                                                aux.additionalData = getBlockLightAverage_(localLight,
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
                                if (x > 0 && (neighborLocalID = blockData_.blocksLocalIDs.get(x - 1, y, z))) {

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
                                                aux.additionalData = getBlockLightAverage_(localLight,
                                                    basicVec3(x, y, z - 1), basicVec3(x, y - 1, z), basicVec3(x, y - 1, z - 1));
                                                aux.lightExtraData.x = 0;
                                                aux.lightExtraData.y = 0;
                                                break;
                                            case 1: // block vertex 2 (C)
                                            case 4:
                                                aux.additionalData = getBlockLightAverage_(localLight,
                                                    basicVec3(x, y, z - 1), basicVec3(x, y + 1, z), basicVec3(x, y + 1, z - 1));
                                                aux.lightExtraData.x = 0;
                                                aux.lightExtraData.y = 1;
                                                break;
                                            case 2: // block vertex 5 (A)
                                            case 3:
                                                aux.additionalData = getBlockLightAverage_(localLight,
                                                    basicVec3(x, y, z + 1), basicVec3(x, y - 1, z), basicVec3(x, y - 1, z + 1));
                                                aux.lightExtraData.x = 1;
                                                aux.lightExtraData.y = 0;
                                                break;
                                            case 5: // block vertex 6 (D)
                                                aux.additionalData = getBlockLightAverage_(localLight,
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
                                if (y < CHUNK_SIZE_LIMIT && (neighborLocalID = blockData_.blocksLocalIDs.get(x, y + 1, z))) {

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
                                                aux.additionalData = getBlockLightAverage_(localLight,
                                                    basicVec3(x + 1, y, z), basicVec3(x, y, z - 1), basicVec3(x + 1, y, z - 1));
                                                aux.lightExtraData.x = 0;
                                                aux.lightExtraData.y = 0;
                                                break;
                                            case 1: // block vertex 5 (C)
                                            case 4:
                                                aux.additionalData = getBlockLightAverage_(localLight,
                                                    basicVec3(x + 1, y, z), basicVec3(x, y, z + 1), basicVec3(x + 1, y, z + 1));
                                                aux.lightExtraData.x = 0;
                                                aux.lightExtraData.y = 1;
                                                break;
                                            case 2: // block vertex 0 (A)
                                            case 3:
                                                aux.additionalData = getBlockLightAverage_(localLight,
                                                    basicVec3(x - 1, y, z), basicVec3(x, y, z - 1), basicVec3(x - 1, y, z - 1));
                                                aux.lightExtraData.x = 1;
                                                aux.lightExtraData.y = 0;
                                                break;
                                            case 5: // block vertex 4 (D)
                                                aux.additionalData = getBlockLightAverage_(localLight,
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
                                if (y > 0 && (neighborLocalID = blockData_.blocksLocalIDs.get(x, y - 1, z))) {

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
                                                aux.additionalData = getBlockLightAverage_(localLight,
                                                    basicVec3(x - 1, y, z), basicVec3(x, y, z - 1), basicVec3(x - 1, y, z - 1));
                                                aux.lightExtraData.x = 0;
                                                aux.lightExtraData.y = 0;
                                                break;
                                            case 1: // block vertex 7 (C)
                                            case 4:
                                                aux.additionalData = getBlockLightAverage_(localLight,
                                                    basicVec3(x - 1, y, z), basicVec3(x, y, z + 1), basicVec3(x - 1, y, z + 1));
                                                aux.lightExtraData.x = 0;
                                                aux.lightExtraData.y = 1;
                                                break;
                                            case 2: // block vertex 2 (A)
                                            case 3:
                                                aux.additionalData = getBlockLightAverage_(localLight,
                                                    basicVec3(x + 1, y, z), basicVec3(x, y, z - 1), basicVec3(x + 1, y, z - 1));
                                                aux.lightExtraData.x = 1;
                                                aux.lightExtraData.y = 0;
                                                break;
                                            case 5: // block vertex 6 (D)
                                                aux.additionalData = getBlockLightAverage_(localLight,
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
                                if (z < CHUNK_SIZE_LIMIT && (neighborLocalID = blockData_.blocksLocalIDs.get(x, y, z + 1))) {

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
                                                aux.additionalData = getBlockLightAverage_(localLight,
                                                    basicVec3(x - 1, y, z), basicVec3(x, y - 1, z), basicVec3(x - 1, y - 1, z));
                                                aux.lightExtraData.x = 0;
                                                aux.lightExtraData.y = 0;
                                                break;
                                            case 1: // block vertex 3 (C)
                                            case 4:
                                                aux.additionalData = getBlockLightAverage_(localLight,
                                                    basicVec3(x - 1, y, z), basicVec3(x, y + 1, z), basicVec3(x - 1, y + 1, z));
                                                aux.lightExtraData.x = 0;
                                                aux.lightExtraData.y = 1;
                                                break;
                                            case 2: // block vertex 1 (A)
                                            case 3:
                                                aux.additionalData = getBlockLightAverage_(localLight,
                                                    basicVec3(x + 1, y, z), basicVec3(x, y - 1, z), basicVec3(x + 1, y - 1, z));
                                                aux.lightExtraData.x = 1;
                                                aux.lightExtraData.y = 0;
                                                break;
                                            case 5: // block vertex 2 (D)
                                                aux.additionalData = getBlockLightAverage_(localLight,
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
                                if (z > 0 && (neighborLocalID = blockData_.blocksLocalIDs.get(x, y, z - 1))) {

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
                                                aux.additionalData = getBlockLightAverage_(localLight,
                                                    basicVec3(x + 1, y, z), basicVec3(x, y - 1, z), basicVec3(x + 1, y - 1, z));
                                                aux.lightExtraData.x = 0;
                                                aux.lightExtraData.y = 0;
                                                break;
                                            case 1: // block vertex 6 (C)
                                            case 4:
                                                aux.additionalData = getBlockLightAverage_(localLight,
                                                    basicVec3(x + 1, y, z), basicVec3(x, y + 1, z), basicVec3(x + 1, y + 1, z));
                                                aux.lightExtraData.x = 0;
                                                aux.lightExtraData.y = 1;
                                                break;
                                            case 2: // block vertex 4 (A)
                                            case 3:
                                                aux.additionalData = getBlockLightAverage_(localLight,
                                                    basicVec3(x - 1, y, z), basicVec3(x, y - 1, z), basicVec3(x - 1, y - 1, z));
                                                aux.lightExtraData.x = 1;
                                                aux.lightExtraData.y = 0;
                                                break;
                                            case 5: // block vertex 7 (D)
                                                aux.additionalData = getBlockLightAverage_(localLight,
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

                        localID = blockData_.blocksLocalIDs.get(CHUNK_SIZE_LIMIT, y, z);
                        const blockLight& localLight = 
                            blockData_.blockLightColor ? blockData_.blockLightColor->get(CHUNK_SIZE_LIMIT, y, z) : blockLightZero;
                        const block& b = localID ? block::getBlockC(palette_.getT2(localID)) : block::emptyBlock();

                        if (b.opacity() <= blockOpacity::TRANSLUCENTBLOCK && (neighborLocalID = blockData_.blocksLocalIDs.get(CHUNK_SIZE, y, z))) {

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
                                        aux.additionalData = getBlockLightAverage_(localLight,
                                            basicVec3(CHUNK_SIZE_LIMIT, y, z + 1), basicVec3(CHUNK_SIZE_LIMIT, y - 1, z), basicVec3(CHUNK_SIZE_LIMIT, y - 1, z + 1));
                                        aux.lightExtraData.x = 0;
                                        aux.lightExtraData.y = 0;
                                        break;
                                    case 1: // block vertex 7 (C)
                                    case 4:
                                        aux.additionalData = getBlockLightAverage_(localLight,
                                            basicVec3(CHUNK_SIZE_LIMIT, y, z + 1), basicVec3(CHUNK_SIZE_LIMIT, y + 1, z), basicVec3(CHUNK_SIZE_LIMIT, y + 1, z + 1));
                                        aux.lightExtraData.x = 0;
                                        aux.lightExtraData.y = 1;
                                        break;
                                    case 2: // block vertex 0 (A)
                                    case 3:
                                        aux.additionalData = getBlockLightAverage_(localLight,
                                            basicVec3(CHUNK_SIZE_LIMIT, y, z - 1), basicVec3(CHUNK_SIZE_LIMIT, y - 1, z), basicVec3(CHUNK_SIZE_LIMIT, y - 1, z - 1));
                                        aux.lightExtraData.x = 1;
                                        aux.lightExtraData.y = 0;
                                        break;
                                    case 5: // block vertex 3 (D)
                                        aux.additionalData = getBlockLightAverage_(localLight,
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

                        localID = blockData_.blocksLocalIDs.get(0, y, z);
                        const blockLight& localLight = 
                            blockData_.blockLightColor ? blockData_.blockLightColor->get(0, y, z) : blockLightZero;
                        const block& b = localID ? block::getBlockC(palette_.getT2(localID)) : block::emptyBlock();

                        if (b.opacity() <= blockOpacity::TRANSLUCENTBLOCK && (neighborLocalID = blockData_.blocksLocalIDs.get(-1, y, z))) {

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
                                        aux.additionalData = getBlockLightAverage_(localLight,
                                            basicVec3(0, y, z - 1), basicVec3(0, y - 1, z), basicVec3(0, y - 1, z - 1));
                                        aux.lightExtraData.x = 0;
                                        aux.lightExtraData.y = 0;
                                        break;
                                    case 1: // block vertex 2 (C)
                                    case 4:
                                        aux.additionalData = getBlockLightAverage_(localLight,
                                            basicVec3(0, y, z - 1), basicVec3(0, y + 1, z), basicVec3(0, y + 1, z - 1));
                                        aux.lightExtraData.x = 0;
                                        aux.lightExtraData.y = 1;
                                        break;
                                    case 2: // block vertex 5 (A)
                                    case 3:
                                        aux.additionalData = getBlockLightAverage_(localLight,
                                            basicVec3(0, y, z + 1), basicVec3(0, y - 1, z), basicVec3(0, y - 1, z + 1));
                                        aux.lightExtraData.x = 1;
                                        aux.lightExtraData.y = 0;
                                        break;
                                    case 5: // block vertex 6 (D)
                                        aux.additionalData = getBlockLightAverage_(localLight,
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

                        localID = blockData_.blocksLocalIDs.get(x, CHUNK_SIZE_LIMIT, z);
                        const blockLight& localLight = 
                            blockData_.blockLightColor ? blockData_.blockLightColor->get(x, CHUNK_SIZE_LIMIT, z) : blockLightZero;
                        const block& b = localID ? block::getBlockC(palette_.getT2(localID)) : block::emptyBlock();

                        if (b.opacity() <= blockOpacity::TRANSLUCENTBLOCK && (neighborLocalID = blockData_.blocksLocalIDs.get(x, CHUNK_SIZE, z))) {

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
                                        aux.additionalData = getBlockLightAverage_(localLight,
                                            basicVec3(x + 1, CHUNK_SIZE_LIMIT, z), basicVec3(x, CHUNK_SIZE_LIMIT, z - 1), basicVec3(x + 1, CHUNK_SIZE_LIMIT, z - 1));
                                        aux.lightExtraData.x = 0;
                                        aux.lightExtraData.y = 0;
                                        break;
                                    case 1: // block vertex 5 (C)
                                    case 4:
                                        aux.additionalData = getBlockLightAverage_(localLight,
                                            basicVec3(x + 1, CHUNK_SIZE_LIMIT, z), basicVec3(x, CHUNK_SIZE_LIMIT, z + 1), basicVec3(x + 1, CHUNK_SIZE_LIMIT, z + 1));
                                        aux.lightExtraData.x = 0;
                                        aux.lightExtraData.y = 1;
                                        break;
                                    case 2: // block vertex 0 (A)
                                    case 3:
                                        aux.additionalData = getBlockLightAverage_(localLight,
                                            basicVec3(x - 1, CHUNK_SIZE_LIMIT, z), basicVec3(x, CHUNK_SIZE_LIMIT, z - 1), basicVec3(x - 1, CHUNK_SIZE_LIMIT, z - 1));
                                        aux.lightExtraData.x = 1;
                                        aux.lightExtraData.y = 0;
                                        break;
                                    case 5: // block vertex 4 (D)
                                        aux.additionalData = getBlockLightAverage_(localLight,
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

                        localID = blockData_.blocksLocalIDs.get(x, 0, z);
                        const blockLight& localLight = 
                            blockData_.blockLightColor ? blockData_.blockLightColor->get(x, 0, z) : blockLightZero;
                        const block& b = localID ? block::getBlockC(palette_.getT2(localID)) : block::emptyBlock();

                        if (b.opacity() <= blockOpacity::TRANSLUCENTBLOCK && (neighborLocalID = blockData_.blocksLocalIDs.get(x, -1, z))) {

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
                                        aux.additionalData = getBlockLightAverage_(localLight,
                                            basicVec3(x - 1, 0, z), basicVec3(x, 0, z - 1), basicVec3(x - 1, 0, z - 1));
                                        aux.lightExtraData.x = 0;
                                        aux.lightExtraData.y = 0;
                                        break;
                                    case 1: // block vertex 7 (C)
                                    case 4:
                                        aux.additionalData = getBlockLightAverage_(localLight,
                                            basicVec3(x - 1, 0, z), basicVec3(x, 0, z + 1), basicVec3(x - 1, 0, z + 1));
                                        aux.lightExtraData.x = 0;
                                        aux.lightExtraData.y = 1;
                                        break;
                                    case 2: // block vertex 2 (A)
                                    case 3:
                                        aux.additionalData = getBlockLightAverage_(localLight,
                                            basicVec3(x + 1, 0, z), basicVec3(x, 0, z - 1), basicVec3(x + 1, 0, z - 1));
                                        aux.lightExtraData.x = 1;
                                        aux.lightExtraData.y = 0;
                                        break;
                                    case 5: // block vertex 6 (D)
                                        aux.additionalData = getBlockLightAverage_(localLight,
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

                        localID = blockData_.blocksLocalIDs.get(x, y, CHUNK_SIZE_LIMIT);
                        const blockLight& localLight = 
                            blockData_.blockLightColor ? blockData_.blockLightColor->get(x, y, CHUNK_SIZE_LIMIT) : blockLightZero;
                        const block& b = localID ? block::getBlockC(palette_.getT2(localID)) : block::emptyBlock();

                        if (b.opacity() <= blockOpacity::TRANSLUCENTBLOCK && (neighborLocalID = blockData_.blocksLocalIDs.get(x, y, CHUNK_SIZE))) {

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
                                        aux.additionalData = getBlockLightAverage_(localLight,
                                            basicVec3(x - 1, y, CHUNK_SIZE_LIMIT), basicVec3(x, y - 1, CHUNK_SIZE_LIMIT), basicVec3(x - 1, y - 1, CHUNK_SIZE_LIMIT));
                                        aux.lightExtraData.x = 0;
                                        aux.lightExtraData.y = 0;
                                        break;
                                    case 1: // block vertex 3 (C)
                                    case 4:
                                        aux.additionalData = getBlockLightAverage_(localLight,
                                            basicVec3(x - 1, y, CHUNK_SIZE_LIMIT), basicVec3(x, y + 1, CHUNK_SIZE_LIMIT), basicVec3(x - 1, y + 1, CHUNK_SIZE_LIMIT));
                                        aux.lightExtraData.x = 0;
                                        aux.lightExtraData.y = 1;
                                        break;
                                    case 2: // block vertex 1 (A)
                                    case 3:
                                        aux.additionalData = getBlockLightAverage_(localLight,
                                            basicVec3(x + 1, y, CHUNK_SIZE_LIMIT), basicVec3(x, y - 1, CHUNK_SIZE_LIMIT), basicVec3(x + 1, y - 1, CHUNK_SIZE_LIMIT));
                                        aux.lightExtraData.x = 1;
                                        aux.lightExtraData.y = 0;
                                        break;
                                    case 5: // block vertex 2 (D)
                                        aux.additionalData = getBlockLightAverage_(localLight,
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

                        localID = blockData_.blocksLocalIDs.get(x, y, 0);
                        const blockLight& localLight = 
                            blockData_.blockLightColor ? blockData_.blockLightColor->get(x, y, 0) : blockLightZero;
                        const block& b = localID ? block::getBlockC(palette_.getT2(localID)) : block::emptyBlock();

                        if (b.opacity() <= blockOpacity::TRANSLUCENTBLOCK && (neighborLocalID = blockData_.blocksLocalIDs.get(x, y, -1))) {

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
                                        aux.additionalData = getBlockLightAverage_(localLight,
                                            basicVec3(x + 1, y, 0), basicVec3(x, y - 1, 0), basicVec3(x + 1, y - 1, 0));
                                        aux.lightExtraData.x = 0;
                                        aux.lightExtraData.y = 0;
                                        break;
                                    case 1: // block vertex 6 (C)
                                    case 4:
                                        aux.additionalData = getBlockLightAverage_(localLight,
                                            basicVec3(x + 1, y, 0), basicVec3(x, y + 1, 0), basicVec3(x + 1, y + 1, 0));
                                        aux.lightExtraData.x = 0;
                                        aux.lightExtraData.y = 1;
                                        break;
                                    case 2: // block vertex 4 (A)
                                    case 3:
                                        aux.additionalData = getBlockLightAverage_(localLight,
                                            basicVec3(x - 1, y, 0), basicVec3(x, y - 1, 0), basicVec3(x - 1, y - 1, 0));
                                        aux.lightExtraData.x = 1;
                                        aux.lightExtraData.y = 0;
                                        break;
                                    case 5: // block vertex 7 (D)
                                        aux.additionalData = getBlockLightAverage_(localLight,
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

        blockData_.isOpaque.clear();
        blockData_.floodPointLightPositions.clear();
        if (blockData_.blockLightColor)
            blockData_.blockLightColor->clear();

    }

    void chunk::addBlockLight(char x, char y, char z,
        lightIntensity intensity, lightValue value, colorChannel channel,
        bool markToBeRemeshed) {

        if (!blockData_.blockLightColor || intensity > blockData_.blockLightColor->get(x, y, z).intensity(channel))
            setBlockLight(x, y, z, intensity, value, channel, markToBeRemeshed);
    
    }

    void chunk::setBlockLight(const ivec3& inChunkPos,
        const blockLight& color, colorChannel channel,
        bool markToBeRemeshed) {
        
        initBlockLight();
        blockData_.blockLightColor->get(inChunkPos.x, inChunkPos.y, inChunkPos.z).copy(color, channel);

        if (markToBeRemeshed)
            needsRemesh_ = true;

    }

    void chunk::setBlockLight(char x, char y, char z,
        lightIntensity intensity, lightValue value, colorChannel channel,
        bool markToBeRemeshed) {

        initBlockLight();
        blockLight& actualColor = blockData_.blockLightColor->get(x, y, z);

        actualColor.intensity(intensity, channel);
        actualColor.value(value, channel);

        if (markToBeRemeshed)
            needsRemesh_ = true;
    
    }

    void chunk::makeEmpty() {

        {
            std::unique_lock<std::recursive_mutex> lock(loadStatusMutex_);
            loadStatus_ = chunkLoadStatus::NOTLOADED;
        }

        blocksMutex_.lock();

        palette_.clear();
        paletteCount_.clear();
        freeLocalIDs_.clear();
        clearBlockLight();
        blockData_.blocksLocalIDs.clear();

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

    void chunk::disownChunks() {
        
        std::unique_lock<std::recursive_mutex> lock(chunkManager::chunksMutex());
        std::unique_lock<std::recursive_mutex> lock2(ownedChunksMutex_);
            
        for (auto it = ownedChunks_.cbegin(); it != ownedChunks_.cend(); it++)
            if (chunk* c = it->second)
                chunkManager::removeOwner(*c, chunkPos_);
        ownedChunks_.clear();

    }

    void chunk::initBlockLight() {

        if (!blockData_.blockLightColor)
            blockData_.blockLightColor = new Padded3DArray<blockLight>(16, 16, 16, 1, blockLight::zero());

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

    basicVec4 chunk::getBlockLightAverage_(const blockLight& blockLightOwn,
        const basicVec3& blockLightCoords1, const basicVec3& blockLightCoords2, const basicVec3& blockLightCoords3) {

        if (!blockData_.blockLightColor)
            return basicVec4Zero;

        const char AOLight = 0;

        bool isOpaqueCoords1 = blockData_.isOpaque.get(blockLightCoords1.x, blockLightCoords1.y, blockLightCoords1.z);
        bool isOpaqueCoords2 = blockData_.isOpaque.get(blockLightCoords2.x, blockLightCoords2.y, blockLightCoords2.z);
        bool isOpaqueCoords3 = blockData_.isOpaque.get(blockLightCoords3.x, blockLightCoords3.y, blockLightCoords3.z);
        const blockLight& colorCoords1 = blockData_.blockLightColor->get(blockLightCoords1.x, blockLightCoords1.y, blockLightCoords1.z);
        const blockLight& colorCoords2 = blockData_.blockLightColor->get(blockLightCoords2.x, blockLightCoords2.y, blockLightCoords2.z);
        const blockLight& colorCoords3 = blockData_.blockLightColor->get(blockLightCoords3.x, blockLightCoords3.y, blockLightCoords3.z);
        lightValue redValues[4] = { 
            blockLightOwn.getWithIntensity(colorChannel::RED), colorCoords1.getWithIntensity(colorChannel::RED), 
            colorCoords2.getWithIntensity(colorChannel::RED),  colorCoords3.getWithIntensity(colorChannel::RED) };
        lightValue greenValues[4] = {
            blockLightOwn.getWithIntensity(colorChannel::GREEN), colorCoords1.getWithIntensity(colorChannel::GREEN),
            colorCoords2.getWithIntensity(colorChannel::GREEN),  colorCoords3.getWithIntensity(colorChannel::GREEN) };
        lightValue blueValues[4] = {
            blockLightOwn.getWithIntensity(colorChannel::BLUE), colorCoords1.getWithIntensity(colorChannel::BLUE),
            colorCoords2.getWithIntensity(colorChannel::BLUE),  colorCoords3.getWithIntensity(colorChannel::BLUE) };
        lightValue alphaValues[4] = {
            blockLightOwn.getWithIntensity(colorChannel::ALPHA), colorCoords1.getWithIntensity(colorChannel::ALPHA),
            colorCoords2.getWithIntensity(colorChannel::ALPHA),  colorCoords3.getWithIntensity(colorChannel::ALPHA) };

        float red = (redValues[0] +
            (isOpaqueCoords1 ? AOLight : redValues[1]) +
            (isOpaqueCoords2 ? AOLight : redValues[2]) +
            (isOpaqueCoords3 ? AOLight : redValues[3])) / 4.0f;
        float green = (greenValues[0] +
            (isOpaqueCoords1 ? AOLight : greenValues[1]) +
            (isOpaqueCoords2 ? AOLight : greenValues[2]) +
            (isOpaqueCoords3 ? AOLight : greenValues[3])) / 4.0f;
        float blue = (blueValues[0] +
            (isOpaqueCoords1 ? AOLight : blueValues[1]) +
            (isOpaqueCoords2 ? AOLight : blueValues[2]) +
            (isOpaqueCoords3 ? AOLight : blueValues[3])) / 4.0f;
        float alpha = (alphaValues[0] +
            (isOpaqueCoords1 ? AOLight : alphaValues[1]) +
            (isOpaqueCoords2 ? AOLight : alphaValues[2]) +
            (isOpaqueCoords3 ? AOLight : alphaValues[3])) / 4.0f;

        return basicVec4(red, green, blue, alpha);

    }

    void chunk::getAndOwnChunksWithOffset_(int negOffsetX, int negOffsetY, int negOffsetZ, 
        int offsetX, int offsetY, int offsetZ, bool relativeCoords) {

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

    std::mutex chunkManager::managerThreadMutex_;
    std::mutex chunkManager::priorityManagerThreadMutex_;
    std::mutex chunkManager::priorityNewChunkMeshesMutex_;
    std::mutex chunkManager::priorityUpdatesRemainingMutex_;
    std::mutex chunkManager::loadingTerrainMutex_;
    std::recursive_mutex chunkManager::newChunkMeshesMutex_;
    std::recursive_mutex chunkManager::chunksMutex_;
    std::condition_variable chunkManager::managerThreadCV_;
    std::condition_variable chunkManager::priorityManagerThreadCV_;
    std::condition_variable chunkManager::priorityNewChunkMeshesCV_;
    std::condition_variable chunkManager::loadingTerrainCV_;

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
    atomicJobPool* chunkManager::lightJobs_ = nullptr;
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

            lightJobs_ = new atomicJobPool(1); // Only one thread for processing block lighting updates.
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
                    int b = 3 + 2; // TODO. HAY CHUNKS QUE NO SE DESCARGAN NUNCA PORQUE SE HAN QUEDADO EN SIMULATED CON OWNERS COLGANDO. PRUEBA A METER LOS OWNERS EN UN STD::MAP MEJOR.

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
                    //c->poraqui23 = true;

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
        return ivec3{ std::abs(chunkPos.x - playerPos.x),
                      std::abs(chunkPos.y - playerPos.y),
                      std::abs(chunkPos.z - playerPos.z) };

    }

    ivec3 chunkManager::chunkDistance(const ivec3& chunkPos1, const ivec3& chunkPos2) {

        return abs(chunkSignedDistance(chunkPos1, chunkPos2));

    }

    ivec3 chunkManager::chunkSignedDistance(const ivec3& chunkPos1, const ivec3& chunkPos2) {

        return ivec3{ chunkPos1.x - chunkPos2.x, chunkPos1.y - chunkPos2.y, chunkPos1.z - chunkPos2.z };

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
        {

            std::unique_lock<std::recursive_mutex> lock(chunksMutex_);
            auto it = clientChunks_.find(chunkPos);
            if (it == clientChunks_.cend())
                logger::errorLog("Chunk " + std::to_string(chunkPos.x) + "|" + std::to_string(chunkPos.y) + "|" + std::to_string(chunkPos.z) + " does not exist");
            else
                return it->second->setBlock(getChunkRelCoords(x, y, z), blockID);

        }

    }

    chunk* chunkManager::neighborMinusX(const ivec3& chunkPos) {

        ivec3 neighborChunkPos{ chunkPos.x - 1, chunkPos.y, chunkPos.z };
        {
        
            std::unique_lock<std::recursive_mutex> lock(chunksMutex_);
            auto it = clientChunks_.find(neighborChunkPos);
            if (it != clientChunks_.end())
                return it->second;
            else
                return nullptr;
        
        }

    }

    chunk* chunkManager::neighborPlusX(const ivec3& chunkPos) {

        ivec3 neighborChunkPos{ chunkPos.x + 1, chunkPos.y, chunkPos.z };
        {
        
            std::unique_lock<std::recursive_mutex> lock(chunksMutex_);
            auto it = clientChunks_.find(neighborChunkPos);
            if (it != clientChunks_.end())
                return it->second;
            else
                return nullptr;
        
        }

    }

    chunk* chunkManager::neighborMinusY(const ivec3& chunkPos) {

        ivec3 neighborChunkPos{ chunkPos.x, chunkPos.y - 1, chunkPos.z };
        {
        
            std::unique_lock<std::recursive_mutex> lock(chunksMutex_);
            auto it = clientChunks_.find(neighborChunkPos);
            if (it != clientChunks_.end())
                return it->second;
            else
                return nullptr;
        
        }

    }

    chunk* chunkManager::neighborPlusY(const ivec3& chunkPos) {

        ivec3 neighborChunkPos{ chunkPos.x, chunkPos.y + 1, chunkPos.z };
        {
        
            std::unique_lock<std::recursive_mutex> lock(chunksMutex_);
            auto it = clientChunks_.find(neighborChunkPos);
            if (it != clientChunks_.end())
                return it->second;
            else
                return nullptr;
        
        }

    }

    chunk* chunkManager::neighborMinusZ(const ivec3& chunkPos) {

        ivec3 neighborChunkPos{ chunkPos.x, chunkPos.y, chunkPos.z - 1 };
        {
        
            std::unique_lock<std::recursive_mutex> lock(chunksMutex_);
            auto it = clientChunks_.find(neighborChunkPos);
            if (it != clientChunks_.end())
                return it->second;
            else
                return nullptr;
        
        }

    }

    chunk* chunkManager::neighborPlusZ(const ivec3& chunkPos) {

        ivec3 neighborChunkPos{ chunkPos.x, chunkPos.y, chunkPos.z + 1 };
        {
        
            std::unique_lock<std::recursive_mutex> lock(chunksMutex_);
            auto it = clientChunks_.find(neighborChunkPos);
            if (it != clientChunks_.end())
                return it->second;
            else
                return nullptr;
        
        }

    }

    void chunkManager::waitInitialTerrainLoaded() {

        std::unique_lock<std::mutex> lock(loadingTerrainMutex_);
        while (waitInitialTerrainLoaded_)
            loadingTerrainCV_.wait(lock);

    }

    void chunkManager::unloadFrontierChunk(const ivec3& chunkPos) {

        chunk* c = nullptr;
        {
            std::unique_lock<std::recursive_mutex> lock(chunksMutex_);
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
                //c->poraqui17 = true;

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
                    //c->poraqui18 = true;

                }
                else {

                    //c->poraqui19 = true;
                    decreaseNeighborInfoPassCounter_(*c);
                    issueChunkJob(chunkJobType::UNLOADANDSAVE, c, true);

                }

            }
        }

    }

    void chunkManager::onUnloadAsFrontier(const ivec3& chunkPos) {

        if (!frontierChunksSet_.contains(chunkPos) && chunkExists_(chunkPos) != chunkExistence::NOEXISTS) {
        
            //if (clientChunks_.contains(chunkPos))
                //clientChunks_[chunkPos]->poraqui26 = true;
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

                        //c.poraqui14 = true;
                        //c.poraqui15 = c.loadStatus();

                        if (chunkInRenderDistance(chunkPos)) {

                            //c.poraqui29 = true;
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

            job& aJob = lightJobs_->get();

            // Set the task.
            std::tuple<chunk*, bool>* data = new std::tuple<chunk*, bool>(c, causesPriorityUpdate);
            aJob.pushBackTask(processLightJob, data);

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
                //c->poraqui13 = true;
            
            }
            else {
            
                clientChunks_[chunkPos] = c;
                //c->poraqui12 = true;

            }
                
            if (ownNeighborChunks) {
            
                c->getAndOwnChunks();
                //c->poraqui11 = true;
            
            }
            //else
                //c->poraqui10 = true;
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

        bool chunkEnsured = false;
        chunk* c = getChunk_(chunkPos);
        //if (c)
            //c->poraqui25 = true;

        if (chunkInRenderDistance(chunkPos)) {

            chunkExistence existence = chunkExistence::NOEXISTS;
            c = getChunk_(chunkPos, existence, true);
            //c->poraqui2 = true;

            switch (existence) {
            
            case chunkExistence::SIMULATED:
                simulatedChunkToCommon_(c);
                chunkEnsured = true;
                break;
            case chunkExistence::COMMON:
                //c->poraqui3 = true;
                c->loadStatusMutex().lock();
                if (c->loadStatus() == chunkLoadStatus::NOTLOADED) {
                    c->loadStatusMutex().unlock();

                    loadOrRemesh_(c);

                }
                else
                    c->loadStatusMutex().unlock();

                chunkEnsured = true;
                break;

            case chunkExistence::NOEXISTS:
                break;

            default:
                logger::errorLog("Unsupported chunkExistence enum " + static_cast<int>(existence));
                break;
            
            }

        }

        return chunkEnsured;

    }

    std::string chunkManager::serializeChunk(chunk* c) {

        timer t;
        t.start();

        c->blocksDataMutex().lock_shared();

        // Save local ID data (including duplicated data about neighbors).
        const chunkBlockData& cBlockData = c->blockData();
        const Padded3DArray<unsigned short>& blocks = c->blocks();
        std::string data(c->blocks().size() * sizeof(unsigned short), 0);
        std::memcpy(data.data(), c->blocks().data(), c->blocks().size() * sizeof(unsigned short));

        data += '@';

        std::string isOpaqueData(cBlockData.isOpaque.size() * sizeof(bool), 0);
        std::memcpy(isOpaqueData.data(), cBlockData.isOpaque.data(), cBlockData.isOpaque.size() * sizeof(bool));

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
        unsigned int index = 0;
        unsigned int nBytes = data.size();
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

        std::memcpy(cBlockData.isOpaque.data(), dataBegin + index, cBlockData.isOpaque.size() * sizeof(bool));
        index += cBlockData.isOpaque.size() * sizeof(bool) + 1; // +1 to skip the '@' delimiter character.

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

            // MAÑANA. QUEDA PENDIENTE EL SAVE Y LOAD DE LUCES.
            unsigned short localID = cBlockData.blocksLocalIDs.at(x,y,z);
            unsigned int globalID = localID ? chunkPalette.getT2(localID) : 0;
            //chunk->setBlockLight(block::getBlockC(globalID), pos);

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
            //c->poraqui8 = true;
        
        }
        else
        {
            c->loadStatusMutex().unlock();
            //c->poraqui5 = true;
        }
           
        return c;

    }

    void chunkManager::issueChunkJob(const std::list<std::pair<chunkJobType, void*>>& jobsData, bool pushJobBack) {

        bool shouldSendJob = true;
        bool priorityJob = false;
        job& aJob = loadChunkJobs_->get();
        for (auto it = jobsData.begin(); shouldSendJob && it != jobsData.end(); it++) {
        
            shouldSendJob = pushChunkTask_(it->first, it->second, aJob);
            priorityJob = priorityJob || isPriority(it->first);
        
        }

        if (shouldSendJob)
            sendJob_(aJob, pushJobBack, priorityJob);

    }
    void chunkManager::issueChunkJob(chunkJobType type, void* data, bool pushJobBack) {

        job& aJob = loadChunkJobs_->get();

        // Send the job to its corresponding queue if adequate.
        if (pushChunkTask_(type, data, aJob))
            sendJob_(aJob, pushJobBack, isPriority(type));
        else
            loadChunkJobs_->free(aJob);
        
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

            //c->poraqui4 = true;

            existence = chunkExistence::COMMON;
            return c;

        }
        else if (foundSimulatedChunk) {

            //c->poraqui6 = true;

            existence = chunkExistence::SIMULATED;
            return c;

        }
        else {

            existence = chunkExistence::NOEXISTS;

            if (createIfDoesntExist) {

                existence = chunkExistence::COMMON;
                c = loadFrontierChunk(chunkPos);
                //c->poraqui7 = true;
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

        //c->poraqui20 = true;

        if (isPriorityUpdate) {

            //c->poraqui21 = true;
            priorityNewChunkMeshesMutex_.lock();
            priorityNewChunkMeshes_.push_back(c);
            priorityNewChunkMeshesMutex_.unlock();

            priorityNewChunkMeshesCV_.notify_all();

        }
        else if (meshSize) {

            //c->poraqui22 = true;
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
            //c->poraqui = true;

        }
        else {
            c->loadStatusMutex().unlock();

            //c->poraqui9 = true;

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
            //if(neighbor)
               // neighbor->poraqui28 = true;
            infoNeighbor->neighborsGenPass1Completed_.unlock();

        }

        //c->poraqui27 = true;

    }

    void chunkManager::decreaseNeighborInfoPassCounter_(chunk& c) {

        // Remove neighborInfo data originating from this chunk.
        const ivec3& chunkPos = c.chunkPos();

        std::unordered_map<ivec3, std::shared_ptr<neighborsInfo>>::iterator info;
        bool thereIsInfo = false;
        {
            std::unique_lock<std::mutex> lock(chunkNeighborsInfoMutex_);
            info = chunkNeighborsInfo_.find(chunkPos);
            thereIsInfo = info != chunkNeighborsInfo_.cend();
        }
        
        if (thereIsInfo) {

            std::shared_ptr<neighborsInfo> n = info->second;
            n->neighborsGenPass1Completed_.lock();
            std::set<ivec3>& completed = n->neighborsGenPass1Completed_.get();
            completed.erase(chunkPos);
            if (completed.empty()) {

                n->neighborsGenPass1Completed_.unlock();

                {
                    std::unique_lock<std::mutex> lock(chunkNeighborsInfoMutex_);
                    chunkNeighborsInfo_.erase(chunkPos); // TODO. REUSABLE NEIGHBORINFO OBJECTS???
                }

            }
            else
                n->neighborsGenPass1Completed_.unlock();

            ivec3 neighborPos;
            for (const ivec3& offset : neighborsOffsets) {

                neighborPos = chunkPos + offset;

                {
                    std::unique_lock<std::mutex> lock(chunkNeighborsInfoMutex_);
                    info = chunkNeighborsInfo_.find(neighborPos);
                    thereIsInfo = info != chunkNeighborsInfo_.cend();
                }

                if (thereIsInfo)
                {
                    std::shared_ptr<neighborsInfo> neighborInfo = info->second;
                    neighborInfo->neighborsGenPass1Completed_.lock();
                    std::set<ivec3>& neighborCompleted = neighborInfo->neighborsGenPass1Completed_.get();
                    neighborCompleted.erase(neighborPos);
                    if (neighborCompleted.empty()) {

                        neighborInfo->neighborsGenPass1Completed_.unlock();

                        {
                            std::unique_lock<std::mutex> lock(chunkNeighborsInfoMutex_);
                            chunkNeighborsInfo_.erase(chunkPos); // TODO. REUSABLE NEIGHBORINFO OBJECTS???
                        }

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

    void chunkManager::recalculateBlockLight_(chunk& c, bool priorityUpdate, const std::unordered_set<ivec3>* lights) {

        std::unique_lock<std::recursive_mutex> lock(c.ownedChunksMutex());
        std::unordered_map<ivec3, chunk*>& ownedChunks = c.ownedChunks();

        if (!ownedChunks.empty()) {

            // Propagate every light across the chunk and its neighbors without taking into account chunk borders.
            if (getDataForCalculatingBlockLight_(c, ownedChunks)) {

                // This is for the flooding process not to end on the same blocklight that starts it if the block is opaque.
                bool firstSecondLoopIteration = false;
                ivec3 chunkPosOffset = vec3Zero;
                ivec3 chunkRelPos = vec3Zero;
                ivec3 neighborOffset = vec3Zero;
                ivec3 neighborPos = vec3Zero;
                ivec3 neighborRelPos = vec3Zero;
                std::deque<blockLightMod> floodLightsInstances;

                // Use provided lights if present or use the chunk's lights otherwise.
                const std::unordered_set<ivec3>& floodPointLightPositions = lights ? *lights : c.getFloodPointLightPositions();

                for (auto it = floodPointLightPositions.cbegin(); it != floodPointLightPositions.cend(); it++) {

                    // Reset first loop variables.
                    floodLightsInstances.clear();

                    // Add block's light and init second loop variables.
                    const ivec3* blockGlobalPos = &*it; // We assume the chunk is the center of the coordinate system.
                    chunkPosOffset = getChunkCoords(*blockGlobalPos);
                    chunkRelPos = getChunkRelCoords(*blockGlobalPos);
                    const chunkBlockData* blockData = &ownedChunks[chunkPosOffset]->blockData(); // TODO. AÑADIR FUNCION SWITCH PARA NO USAR EL HASH DE DICCIONARIO
                    floodLightsInstances.emplace_back(*blockGlobalPos, 
                        blockData->blockLightColor ? blockData->blockLightColor->get(chunkRelPos) : blockLight::zero());

                    firstSecondLoopIteration = true; // To spread beyond the block light source's position.
                    blockLightMod* floodLight = &floodLightsInstances.front();
                    do {

                        // MAÑANA. NO SE ESTÁ RECALCULANDO EL IS OPAQUE CUANDO SE DESERIALIZA UN CHUNK GUARDADO
                        if (firstSecondLoopIteration || !blockData->isOpaque.at(chunkRelPos)) {

                            addLightChannelSource_(colorChannel::RED, *floodLight, ownedChunks,
                                chunkPosOffset, chunkRelPos, floodLightsInstances, *blockGlobalPos, firstSecondLoopIteration);
                            addLightChannelSource_(colorChannel::GREEN, *floodLight, ownedChunks,
                                chunkPosOffset, chunkRelPos, floodLightsInstances, *blockGlobalPos, firstSecondLoopIteration);
                            addLightChannelSource_(colorChannel::BLUE, *floodLight, ownedChunks,
                                chunkPosOffset, chunkRelPos, floodLightsInstances, *blockGlobalPos, firstSecondLoopIteration);
                            addLightChannelSource_(colorChannel::ALPHA, *floodLight, ownedChunks,
                                chunkPosOffset, chunkRelPos, floodLightsInstances, *blockGlobalPos, firstSecondLoopIteration);

                        }
                        floodLightsInstances.pop_front();

                        if (floodLightsInstances.size() > 0) {

                            // Reset loop variables
                            firstSecondLoopIteration = false;
                            floodLight = &floodLightsInstances.front();
                            blockGlobalPos = &floodLight->pos; // We assume the chunk is the center of the coordinate system.
                            chunkPosOffset = getChunkCoords(*blockGlobalPos);
                            chunkRelPos = getChunkRelCoords(*blockGlobalPos);
                            blockData = &ownedChunks[chunkPosOffset]->blockData(); // TODO. AÑADIR FUNCION SWITCH PARA NO USAR EL HASH DE DICCIONARIO

                        }

                    } while (floodLightsInstances.size() > 0);

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

    bool chunkManager::getDataForCalculatingBlockLight_(chunk& c, const std::unordered_map<ivec3, chunk*>& ownedChunks) {

        bool isDataEnough = true;

        //std::unique_lock<std::recursive_mutex> lock(chunksMutex_); // Careful! This causes deadlock.

        for (auto it = neighborsOffsets.cbegin(); (isDataEnough = ownedChunks.at(*it)) && it != neighborsOffsets.cend(); it++);

        return isDataEnough;

    }

    void chunkManager::addLightChannelSource_(colorChannel channel, blockLightMod& floodLight, std::unordered_map<ivec3, chunk*>& ownedChunks,
        const ivec3& chunkPosOffset, const ivec3& chunkRelPos, std::deque<blockLightMod>& floodLightsInstances, const ivec3& blockGlobalPos, 
        bool ignoreIntensityComparison) {

        lightIntensity intensity = floodLight.color.intensity(channel);

        if (intensity) {
        
            lightValue value = floodLight.color.value(channel);

            chunk* ownedChunk = ownedChunks[chunkPosOffset];
            ownedChunk->initBlockLight();

            Padded3DArray<blockLight>* blockLightColor = ownedChunk->blockData().blockLightColor;
            blockLight* intensityCache = &blockLightColor->get(chunkRelPos.x, chunkRelPos.y, chunkRelPos.z);
            lightIntensity currentIntensity = intensityCache->intensity(channel); // Current intensity of the block neighbor to expand to in the "BDF" algorithm.

            if (ignoreIntensityComparison || intensity > currentIntensity) {

                // Apply light on position.
                ownedChunk->setBlockLight(chunkRelPos, floodLight.color, channel);
                intensityCache->intensity(intensity, channel);

                // Calculate light blending across chunks.
                lightIntensity intensityMinus1 = intensity - 1;
                chunk* neighborChunk = nullptr;
                ivec3 neighborOffset(
                    chunkRelPos.x >= CHUNK_SIZE_LIMIT ? 1 : chunkRelPos.x <= 0 ? -1 : 0,
                    chunkRelPos.y >= CHUNK_SIZE_LIMIT ? 1 : chunkRelPos.y <= 0 ? -1 : 0,
                    chunkRelPos.z >= CHUNK_SIZE_LIMIT ? 1 : chunkRelPos.z <= 0 ? -1 : 0
                );
                ivec3 neighborPos;
                ivec3 neighborRelPos;
                if (neighborOffset.x != 0) {

                    if (neighborOffset.y != 0) {

                        // X Y Z
                        if (neighborOffset.z != 0) {

                            neighborPos = chunkPosOffset + neighborOffset;
                            neighborRelPos = getChunkRelCoords(chunkRelPos + neighborOffset);
                            neighborChunk = ownedChunks[neighborPos];
                            currentIntensity = neighborChunk->getBlockLight(neighborRelPos).intensity(channel);

                            if (intensityMinus1 >= currentIntensity) {

                                // Apply light to both neighbors so that smooth lighting's blending can be made correctly across chunks.
                                ownedChunk->setBlockLight(chunkRelPos + neighborOffset,
                                    intensityMinus1, value, channel);

                                neighborChunk->setBlockLight(neighborRelPos - neighborOffset,
                                    intensity, value, channel);

                            }

                        }

                        // X Y 0
                        neighborPos = ivec3(chunkPosOffset.x + neighborOffset.x, chunkPosOffset.y + neighborOffset.y, chunkPosOffset.z);
                        neighborRelPos =
                            getChunkRelCoords(ivec3(chunkRelPos.x + neighborOffset.x, chunkRelPos.y + neighborOffset.y, chunkRelPos.z));
                        neighborChunk = ownedChunks[neighborPos];
                        currentIntensity = neighborChunk->getBlockLight(neighborRelPos).intensity(channel);

                        if (intensityMinus1 >= currentIntensity) {

                            // Apply light to both neighbors so that smooth lighting's blending can be made correctly.
                            ownedChunk->setBlockLight(chunkRelPos, neighborOffset.x, neighborOffset.y, 0,
                                intensityMinus1, value, channel);

                            neighborChunk->setBlockLight(neighborRelPos, -neighborOffset.x, -neighborOffset.y, 0,
                                intensity, value, channel);

                        }

                    }

                    // X 0 Z
                    if (neighborOffset.z != 0) {

                        neighborPos = ivec3(chunkPosOffset.x + neighborOffset.x, chunkPosOffset.y, chunkPosOffset.z + neighborOffset.z);
                        neighborRelPos =
                            getChunkRelCoords(ivec3(chunkRelPos.x + neighborOffset.x, chunkRelPos.y, chunkRelPos.z + neighborOffset.z));
                        neighborChunk = ownedChunks[neighborPos];
                        currentIntensity = neighborChunk->getBlockLight(neighborRelPos).intensity(channel);

                        if (intensityMinus1 >= currentIntensity) {

                            // Apply light to both neighbors so that smooth lighting's blending can be made correctly.
                            ownedChunk->setBlockLight(chunkRelPos, neighborOffset.x, 0, neighborOffset.z,
                                intensityMinus1, value, channel);

                            neighborChunk->setBlockLight(neighborRelPos, -neighborOffset.x, 0, -neighborOffset.z,
                                intensity, value, channel);

                        }

                    }

                    // X 0 0
                    neighborPos = ivec3(chunkPosOffset.x + neighborOffset.x, chunkPosOffset.y, chunkPosOffset.z);
                    neighborRelPos = getChunkRelCoords(ivec3(chunkRelPos.x + neighborOffset.x, chunkRelPos.y, chunkRelPos.z));
                    neighborChunk = ownedChunks[neighborPos];
                    currentIntensity = neighborChunk->getBlockLight(neighborRelPos).intensity(channel);

                    if (intensityMinus1 >= currentIntensity) {

                        // Apply light to both neighbors so that smooth lighting's blending can be made correctly.
                        ownedChunk->setBlockLight(chunkRelPos, neighborOffset.x, 0, 0,
                            intensityMinus1, value, channel);

                        neighborChunk->setBlockLight(neighborRelPos, -neighborOffset.x, 0, 0,
                            intensity, value, channel);

                    }

                }

                if (neighborOffset.y != 0) {

                    // 0 Y Z
                    if (neighborOffset.z != 0) {

                        neighborPos = ivec3(chunkPosOffset.x, chunkPosOffset.y + neighborOffset.y, chunkPosOffset.z + neighborOffset.z);
                        neighborRelPos = getChunkRelCoords(ivec3(chunkRelPos.x, chunkRelPos.y + neighborOffset.y, chunkRelPos.z + neighborOffset.z));
                        neighborChunk = ownedChunks[neighborPos];
                        currentIntensity = neighborChunk->getBlockLight(neighborRelPos).intensity(channel);

                        if (intensityMinus1 >= currentIntensity) {

                            // Apply light to both neighbors so that smooth lighting's blending can be made correctly.
                            ownedChunk->setBlockLight(chunkRelPos, 0, neighborOffset.y, neighborOffset.z,
                                intensityMinus1, value, channel);

                            neighborChunk->setBlockLight(neighborRelPos, 0, -neighborOffset.y, -neighborOffset.z,
                                intensity, value, channel);

                        }
                    }

                    // 0 Y 0
                    neighborPos = ivec3(chunkPosOffset.x, chunkPosOffset.y + neighborOffset.y, chunkPosOffset.z);
                    neighborRelPos = getChunkRelCoords(ivec3(chunkRelPos.x, chunkRelPos.y + neighborOffset.y, chunkRelPos.z));
                    neighborChunk = ownedChunks[neighborPos];
                    currentIntensity = neighborChunk->getBlockLight(neighborRelPos).intensity(channel);

                    if (intensityMinus1 >= currentIntensity) {

                        // Apply light to both neighbors so that smooth lighting's blending can be made correctly.
                        ownedChunk->setBlockLight(chunkRelPos, 0, neighborOffset.y, 0,
                            intensityMinus1, value, channel);

                        neighborChunk->setBlockLight(neighborRelPos, 0, -neighborOffset.y, 0,
                            intensity, value, channel);

                    }

                }

                // 0 0 Z
                if (neighborOffset.z != 0) {

                    neighborPos = ivec3(chunkPosOffset.x, chunkPosOffset.y, chunkPosOffset.z + neighborOffset.z);
                    neighborRelPos = getChunkRelCoords(ivec3(chunkRelPos.x, chunkRelPos.y, chunkRelPos.z + neighborOffset.z));
                    neighborChunk = ownedChunks[neighborPos];
                    currentIntensity = neighborChunk->getBlockLight(neighborRelPos).intensity(channel);

                    if (intensityMinus1 >= currentIntensity) {

                        // Apply light to both neighbors so that smooth lighting's blending can be made correctly.
                        ownedChunk->setBlockLight(chunkRelPos, 0, 0, neighborOffset.z,
                            intensityMinus1, value, channel);

                        neighborChunk->setBlockLight(neighborRelPos, 0, 0, -neighborOffset.z,
                            intensity, value, channel);

                    }

                }

                // Spread light.
                blockLight newBlockLight = floodLight.color.decreased(colorChannel::ALL);
                //+x
                floodLightsInstances.emplace_back(blockGlobalPos + ivec3FixedNorth, newBlockLight);
                //-x
                floodLightsInstances.emplace_back(blockGlobalPos + ivec3FixedSouth, newBlockLight);
                //+y
                floodLightsInstances.emplace_back(blockGlobalPos + ivec3FixedUp, newBlockLight);
                //-y
                floodLightsInstances.emplace_back(blockGlobalPos + ivec3FixedDown, newBlockLight);
                //+z
                floodLightsInstances.emplace_back(blockGlobalPos + ivec3FixedEast, newBlockLight);
                //-z
                floodLightsInstances.emplace_back(blockGlobalPos + ivec3FixedWest, newBlockLight);

            }
        
        }

    }

    void chunkManager::recalculateBlockLightAfterRemoval_(chunk& c, bool priorityUpdate, std::list<ivec3> blockLightsToRemove) {

        std::unique_lock<std::recursive_mutex> lock(c.ownedChunksMutex());
        std::unordered_map<ivec3, chunk*>& ownedChunks = c.ownedChunks();

        if (!ownedChunks.empty()) {

            // Propagate every light across the chunk and its neighbors without taking into account chunk borders.
            if (getDataForCalculatingBlockLight_(c, ownedChunks)) {

                // This is for the flooding process not to end on the same blocklight that starts it if the block is opaque.
                bool firstSecondLoopIteration = false;

                ivec3 chunkPosOffset = vec3Zero;
                ivec3 chunkRelPos = vec3Zero;
                std::deque<blockLightMod> floodLightsInstances;
                std::unordered_set<ivec3> blockLightsToRepropagate;
                for (auto it = blockLightsToRemove.begin(); it != blockLightsToRemove.end(); it++) {

                    // Reset first loop variables.
                    floodLightsInstances.clear();

                    // Add block's light and init second loop variables.
                    const ivec3* blockGlobalPos = &*it; // We assume chunk c is the center of the coordinate system for this method.
                    chunkPosOffset = getChunkCoords(*blockGlobalPos);
                    chunkRelPos = getChunkRelCoords(*blockGlobalPos);
                    const chunkBlockData* blockData = &ownedChunks[chunkPosOffset]->blockData(); // TODO. AÑADIR FUNCION SWITCH PARA NO USAR EL HASH DE DICCIONARIO
                    floodLightsInstances.emplace_back(*blockGlobalPos, 
                        blockData->blockLightColor ? blockData->blockLightColor->at(chunkRelPos) : blockLight::zero());
                    firstSecondLoopIteration = true;
                    blockLightMod* floodLight = &floodLightsInstances.front();
                    do {

                        // MAÑANA. NO SE ESTÁ RECALCULANDO EL IS OPAQUE CUANDO SE DESERIALIZA UN CHUNK GUARDADO
                        if (firstSecondLoopIteration || !blockData->isOpaque.at(chunkRelPos)) {

                            removeLightChannelSource_(colorChannel::RED, *floodLight, ownedChunks,
                                chunkPosOffset, chunkRelPos, floodLightsInstances, *blockGlobalPos, 
                                blockLightsToRepropagate, true);
                            removeLightChannelSource_(colorChannel::GREEN, *floodLight, ownedChunks,
                                chunkPosOffset, chunkRelPos, floodLightsInstances, *blockGlobalPos, 
                                blockLightsToRepropagate, true);
                            removeLightChannelSource_(colorChannel::BLUE, *floodLight, ownedChunks,
                                chunkPosOffset, chunkRelPos, floodLightsInstances, *blockGlobalPos, 
                                blockLightsToRepropagate, true);
                            removeLightChannelSource_(colorChannel::ALPHA, *floodLight, ownedChunks,
                                chunkPosOffset, chunkRelPos, floodLightsInstances, *blockGlobalPos, 
                                blockLightsToRepropagate, true);

                        }
                        floodLightsInstances.pop_front();

                        if (floodLightsInstances.size() > 0) {

                            // Reset loop variables
                            firstSecondLoopIteration = false;
                            floodLight = &floodLightsInstances.front();
                            blockGlobalPos = &floodLight->pos; // We assume chunk c is the center of the coordinate system for this method.
                            chunkPosOffset = getChunkCoords(*blockGlobalPos);
                            chunkRelPos = getChunkRelCoords(*blockGlobalPos);
                            blockData = &ownedChunks[chunkPosOffset]->blockData(); // TODO. AÑADIR FUNCION SWITCH PARA NO USAR EL HASH DE DICCIONARIO

                        }

                    } while (floodLightsInstances.size() > 0);

                }

                // Repropagate the lights that have been affected by the removal but not have been eliminated.
                recalculateBlockLight_(c, priorityUpdate, &blockLightsToRepropagate);

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

    void chunkManager::removeLightChannelSource_(colorChannel channel, blockLightMod& floodLight, 
        std::unordered_map<ivec3, chunk*>& ownedChunks,
        const ivec3& chunkPosOffset, const ivec3& chunkRelPos, std::deque<blockLightMod>& floodLightsInstances,
        const ivec3& blockGlobalPos, std::unordered_set<ivec3>& blockLightsToRepropagate, bool checkIfNeighborsAreLightSources) {

        lightIntensity intensity = floodLight.color.intensity(channel);
        // MAÑANA. HAY QUE TENER EN CUENTA SI HAY OTRO BLOQUE DE SOURCE DE LIGHT QUE ESTÉ DANDO ESTA INTENSIDAD PARA CUANDO INTESITY=CURRENTINTENSITY

        //logger::debugLog(std::to_string(blockGlobalPos));
        
        if (intensity) {

            chunk* ownedChunk = ownedChunks[chunkPosOffset];
            ownedChunk->initBlockLight();

            Padded3DArray<blockLight>* blockLightColor = ownedChunk->blockData().blockLightColor;
            lightIntensity currentIntensity = blockLightColor->get(chunkRelPos.x, chunkRelPos.y, chunkRelPos.z).intensity(channel);
        
            if (currentIntensity == intensity) {

                // Apply light on position.
                ownedChunk->removeBlockLight(chunkRelPos, channel);

                // Calculate light blending across chunks.
                lightIntensity intensityMinus1 = intensity - 1;
                chunk* neighborChunk = nullptr;
                ivec3 neighborOffset(
                    chunkRelPos.x >= CHUNK_SIZE_LIMIT ? 1 : chunkRelPos.x <= 0 ? -1 : 0,
                    chunkRelPos.y >= CHUNK_SIZE_LIMIT ? 1 : chunkRelPos.y <= 0 ? -1 : 0,
                    chunkRelPos.z >= CHUNK_SIZE_LIMIT ? 1 : chunkRelPos.z <= 0 ? -1 : 0
                );
                ivec3 neighborPos = vec3Zero;
                ivec3 neighborRelPos = vec3Zero;
                if (neighborOffset.x != 0) {

                    if (neighborOffset.y != 0) {

                        // X Y Z
                        if (neighborOffset.z != 0) {

                            neighborPos = chunkPosOffset + neighborOffset;
                            neighborRelPos = getChunkRelCoords(chunkRelPos + neighborOffset);
                            neighborChunk = ownedChunks[neighborPos];
                            currentIntensity = neighborChunk->getBlockLight(neighborRelPos).intensity(channel);

                            if (intensityMinus1 > currentIntensity) {
                            
                                // This light values used for bending across chunk will be properly readded in the repropagation step.
                                ownedChunk->removeBlockLight(chunkRelPos + neighborOffset, channel);
                                neighborChunk->removeBlockLight(neighborRelPos - neighborOffset, channel);
                            
                            }
                            
                        }

                        // X Y 0
                        neighborPos = ivec3(chunkPosOffset.x + neighborOffset.x, chunkPosOffset.y + neighborOffset.y, chunkPosOffset.z);
                        neighborRelPos =
                            getChunkRelCoords(ivec3(chunkRelPos.x + neighborOffset.x, chunkRelPos.y + neighborOffset.y, chunkRelPos.z));
                        neighborChunk = ownedChunks[neighborPos];
                        currentIntensity = neighborChunk->getBlockLight(neighborRelPos).intensity(channel);

                        if (intensityMinus1 > currentIntensity) {
                        
                            ownedChunk->removeBlockLight(chunkRelPos, neighborOffset.x, neighborOffset.y, 0, channel);
                            neighborChunk->removeBlockLight(neighborRelPos, -neighborOffset.x, -neighborOffset.y, 0, channel);
                        
                        }

                    }

                    // X 0 Z
                    if (neighborOffset.z != 0) {

                        neighborPos = ivec3(chunkPosOffset.x + neighborOffset.x, chunkPosOffset.y, chunkPosOffset.z + neighborOffset.z);
                        neighborRelPos =
                            getChunkRelCoords(ivec3(chunkRelPos.x + neighborOffset.x, chunkRelPos.y, chunkRelPos.z + neighborOffset.z));
                        neighborChunk = ownedChunks[neighborPos];
                        currentIntensity = neighborChunk->getBlockLight(neighborRelPos).intensity(channel);

                        if (intensityMinus1 > currentIntensity) {
                        
                            ownedChunk->removeBlockLight(chunkRelPos, neighborOffset.x, 0, neighborOffset.z, channel);
                            neighborChunk->removeBlockLight(neighborRelPos, -neighborOffset.x, 0, -neighborOffset.z, channel);
                        
                        }

                    }

                    // X 0 0
                    neighborPos = ivec3(chunkPosOffset.x + neighborOffset.x, chunkPosOffset.y, chunkPosOffset.z);
                    neighborRelPos = getChunkRelCoords(ivec3(chunkRelPos.x + neighborOffset.x, chunkRelPos.y, chunkRelPos.z));
                    neighborChunk = ownedChunks[neighborPos];
                    currentIntensity = neighborChunk->getBlockLight(neighborRelPos).intensity(channel);

                    if (intensityMinus1 > currentIntensity) {
                    
                        ownedChunk->removeBlockLight(chunkRelPos, neighborOffset.x, 0, 0, channel);
                        neighborChunk->removeBlockLight(neighborRelPos, -neighborOffset.x, 0, 0, channel);
                    
                    }

                }

                if (neighborOffset.y != 0) {

                    // 0 Y Z
                    if (neighborOffset.z != 0) {

                        neighborPos = ivec3(chunkPosOffset.x, chunkPosOffset.y + neighborOffset.y, chunkPosOffset.z + neighborOffset.z);
                        neighborRelPos = getChunkRelCoords(ivec3(chunkRelPos.x, chunkRelPos.y + neighborOffset.y, chunkRelPos.z + neighborOffset.z));
                        neighborChunk = ownedChunks[neighborPos];
                        currentIntensity = neighborChunk->getBlockLight(neighborRelPos).intensity(channel);

                        if (intensityMinus1 > currentIntensity) {
                        
                            ownedChunk->removeBlockLight(chunkRelPos, 0, neighborOffset.y, neighborOffset.z, channel);
                            neighborChunk->removeBlockLight(neighborRelPos, 0, -neighborOffset.y, -neighborOffset.z, channel);
                        
                        }

                    }

                    // 0 Y 0
                    neighborPos = ivec3(chunkPosOffset.x, chunkPosOffset.y + neighborOffset.y, chunkPosOffset.z);
                    neighborRelPos = getChunkRelCoords(ivec3(chunkRelPos.x, chunkRelPos.y + neighborOffset.y, chunkRelPos.z));
                    neighborChunk = ownedChunks[neighborPos];
                    currentIntensity = neighborChunk->getBlockLight(neighborRelPos).intensity(channel);

                    if (intensityMinus1 > currentIntensity) {
                    
                        ownedChunk->removeBlockLight(chunkRelPos, 0, neighborOffset.y, 0, channel);
                        neighborChunk->removeBlockLight(neighborRelPos, 0, -neighborOffset.y, 0, channel);
                    
                    }

                }

                // 0 0 Z
                if (neighborOffset.z != 0) {

                    neighborPos = ivec3(chunkPosOffset.x, chunkPosOffset.y, chunkPosOffset.z + neighborOffset.z);
                    neighborRelPos = getChunkRelCoords(ivec3(chunkRelPos.x, chunkRelPos.y, chunkRelPos.z + neighborOffset.z));
                    neighborChunk = ownedChunks[neighborPos];
                    currentIntensity = neighborChunk->getBlockLight(neighborRelPos).intensity(channel);

                    if (intensityMinus1 > currentIntensity) {
                    
                        ownedChunk->removeBlockLight(chunkRelPos, 0, 0, neighborOffset.z, channel);
                        neighborChunk->removeBlockLight(neighborRelPos, 0, 0, -neighborOffset.z, channel);
                    
                    }
                    
                }

                // Spread 'light removal'
                if (checkIfNeighborsAreLightSources) {
                
                    ivec3 neighborGlobalPos = vec3Zero;
                    blockLight newBlockLight = floodLight.color.decreased(colorChannel::ALL);

                    //+x
                    neighborGlobalPos = blockGlobalPos + ivec3FixedNorth;
                    neighborPos = getChunkCoords(neighborRelPos);
                    neighborRelPos = getChunkRelCoords(neighborGlobalPos); // ESTO LA ESTÁ LIANDO. ESTA PONIENDO LUZ EN POSICION CORRECTA PERO COGIENDO DESDE EL CHUNK ANTERIOR EN EL CASO DE TENER QUE IR DE UN CHUNK A OTRO.
                    neighborChunk = ownedChunks[neighborPos];
                    neighborChunk->initBlockLight();
                    if(neighborChunk->blockData().blockLightColor->at(neighborRelPos).intensity(channel) == kLightMaxIntensity)
                        blockLightsToRepropagate.insert(neighborGlobalPos); // ESTO NO TIENE SENTIDO. AQUI HAY QUE PONER POS GLOBAL
                    else
                        floodLightsInstances.emplace_back(neighborGlobalPos, newBlockLight);

                    //-x
                    neighborGlobalPos = blockGlobalPos + ivec3FixedSouth;
                    neighborPos = getChunkCoords(neighborRelPos);
                    neighborRelPos = getChunkRelCoords(neighborGlobalPos);
                    neighborChunk = ownedChunks[neighborPos];
                    neighborChunk->initBlockLight();
                    if (neighborChunk->blockData().blockLightColor->at(neighborRelPos).intensity(channel) == kLightMaxIntensity)
                        blockLightsToRepropagate.insert(neighborGlobalPos);
                    else
                        floodLightsInstances.emplace_back(neighborGlobalPos, newBlockLight);

                    //+y
                    neighborGlobalPos = blockGlobalPos + ivec3FixedUp;
                    neighborPos = getChunkCoords(neighborRelPos);
                    neighborRelPos = getChunkRelCoords(neighborGlobalPos);
                    neighborChunk = ownedChunks[neighborPos];
                    neighborChunk->initBlockLight();
                    if (neighborChunk->blockData().blockLightColor->at(neighborRelPos).intensity(channel) == kLightMaxIntensity)
                        blockLightsToRepropagate.insert(neighborGlobalPos);
                    else
                        floodLightsInstances.emplace_back(neighborGlobalPos, newBlockLight);

                    //-y
                    neighborGlobalPos = blockGlobalPos + ivec3FixedDown;
                    neighborPos = getChunkCoords(neighborRelPos);
                    neighborRelPos = getChunkRelCoords(neighborGlobalPos);
                    neighborChunk = ownedChunks[neighborPos];
                    neighborChunk->initBlockLight();
                    if (neighborChunk->blockData().blockLightColor->at(neighborRelPos).intensity(channel) == kLightMaxIntensity)
                        blockLightsToRepropagate.insert(neighborGlobalPos);
                    else
                        floodLightsInstances.emplace_back(neighborGlobalPos, newBlockLight);

                    //+z
                    neighborGlobalPos = blockGlobalPos + ivec3FixedEast;
                    neighborPos = getChunkCoords(neighborRelPos);
                    neighborRelPos = getChunkRelCoords(neighborGlobalPos);
                    neighborChunk = ownedChunks[neighborPos];
                    neighborChunk->initBlockLight();
                    if (neighborChunk->blockData().blockLightColor->at(neighborRelPos).intensity(channel) == kLightMaxIntensity)
                        blockLightsToRepropagate.insert(neighborGlobalPos);
                    else
                        floodLightsInstances.emplace_back(neighborGlobalPos, newBlockLight);

                    //-z
                    neighborGlobalPos = blockGlobalPos + ivec3FixedWest;
                    neighborPos = getChunkCoords(neighborRelPos);
                    neighborRelPos = getChunkRelCoords(neighborGlobalPos);
                    neighborChunk = ownedChunks[neighborPos];
                    neighborChunk->initBlockLight();
                    if (neighborChunk->blockData().blockLightColor->at(neighborRelPos).intensity(channel) == kLightMaxIntensity)
                        blockLightsToRepropagate.insert(neighborGlobalPos);
                    else
                        floodLightsInstances.emplace_back(neighborGlobalPos, newBlockLight);
                
                }
                else {
                
                    blockLight newBlockLight = floodLight.color.decreased(colorChannel::ALL);

                    //+x
                    floodLightsInstances.emplace_back(blockGlobalPos + ivec3FixedNorth, newBlockLight);

                    //-x
                    floodLightsInstances.emplace_back(blockGlobalPos + ivec3FixedSouth, newBlockLight);

                    //+y
                    floodLightsInstances.emplace_back(blockGlobalPos + ivec3FixedUp, newBlockLight);

                    //-y
                    floodLightsInstances.emplace_back(blockGlobalPos + ivec3FixedDown, newBlockLight);

                    //+z
                    floodLightsInstances.emplace_back(blockGlobalPos + ivec3FixedEast, newBlockLight);

                    //-z
                    floodLightsInstances.emplace_back(blockGlobalPos + ivec3FixedWest, newBlockLight);
                
                }

            }
            else if (currentIntensity >= intensity)
                blockLightsToRepropagate.insert(blockGlobalPos);

        }
    
    }

    void chunkManager::recalculateBlockLightAfterNonTransparentPlaced_(
        chunk& c, bool priorityUpdate, std::list<ivec3>& blockPositions, const block& placedBlock) {
    
        std::unique_lock<std::recursive_mutex> lock(c.ownedChunksMutex());
        std::unordered_map<ivec3, chunk*>& ownedChunks = c.ownedChunks();

        if (!ownedChunks.empty() && !blockPositions.empty()) {

            // Propagate every light across the chunk and its neighbors without taking into account chunk borders.
            // MAÑANA. FACTORIZE THIS getDataForCalculatingBlockLight_ IN A WAY THAT CAN BE SHARED WITH OTHER TASKS INSIDE THE SAME JOB?
            if (getDataForCalculatingBlockLight_(c, ownedChunks)) { 

                // This is for the flooding process not to end on the same blocklight that starts it if the block is opaque.
                bool firstSecondLoopIteration = true;

                ivec3 chunkPosOffset = vec3Zero;
                ivec3 chunkRelPos = vec3Zero;
                std::unordered_set<ivec3> blockLightsToRepropagate;
                do {

                    // Add block's light and init second loop variables.
                    const ivec3* blockGlobalPos = &blockPositions.front(); // We assume the chunk is the center of the coordinate system.
                    chunkPosOffset = getChunkCoords(*blockGlobalPos);
                    chunkRelPos = getChunkRelCoords(*blockGlobalPos);
                   
                    // MAÑANA. NO SE ESTÁ RECALCULANDO EL IS OPAQUE CUANDO SE DESERIALIZA UN CHUNK GUARDADO
                    if (firstSecondLoopIteration || !ownedChunks[chunkPosOffset]->blockData().isOpaque.at(chunkRelPos)) {

                        removeLightFromPlacedBlock_(colorChannel::RED, ownedChunks, *blockGlobalPos,
                            chunkPosOffset, chunkRelPos, blockPositions, firstSecondLoopIteration,
                            blockLightsToRepropagate, placedBlock);
                        removeLightFromPlacedBlock_(colorChannel::GREEN, ownedChunks, *blockGlobalPos,
                            chunkPosOffset, chunkRelPos, blockPositions, firstSecondLoopIteration,
                            blockLightsToRepropagate, placedBlock);
                        removeLightFromPlacedBlock_(colorChannel::BLUE, ownedChunks, *blockGlobalPos,
                            chunkPosOffset, chunkRelPos, blockPositions, firstSecondLoopIteration,
                            blockLightsToRepropagate, placedBlock);
                        removeLightFromPlacedBlock_(colorChannel::ALPHA, ownedChunks, *blockGlobalPos,
                            chunkPosOffset, chunkRelPos, blockPositions, firstSecondLoopIteration,
                            blockLightsToRepropagate, placedBlock);

                    }
                    blockPositions.pop_front();
                    firstSecondLoopIteration = false;

                } while (blockPositions.size() > 0);

                // Repropagate the lights that have been affected by the removal but not have been eliminated.
                recalculateBlockLight_(c, priorityUpdate, &blockLightsToRepropagate);

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

    void chunkManager::recalculateBlockLightAfterNonTransparentRemoved_(
        chunk& c, bool priorityUpdate, std::list<ivec3>& blockPositions) {

        std::unique_lock<std::recursive_mutex> lock(c.ownedChunksMutex());
        std::unordered_map<ivec3, chunk*>& ownedChunks = c.ownedChunks();

        if (!ownedChunks.empty() && !blockPositions.empty()) {

            // Propagate every light across the chunk and its neighbors without taking into account chunk borders.
            // MAÑANA. FACTORIZE THIS getDataForCalculatingBlockLight_ IN A WAY THAT CAN BE SHARED WITH OTHER TASKS INSIDE THE SAME JOB?
            if (getDataForCalculatingBlockLight_(c, ownedChunks)) {

                // This is for the flooding process not to end on the same blocklight that starts it if the block is opaque.
                bool firstSecondLoopIteration = true;

                ivec3 chunkPosOffset = vec3Zero;
                ivec3 chunkRelPos = vec3Zero;
                std::unordered_set<ivec3> blockLightsToRepropagate;
                do {

                    // Add block's light and init second loop variables.
                    const ivec3* blockGlobalPos = &blockPositions.front(); // We assume the chunk is the center of the coordinate system.
                    chunkPosOffset = getChunkCoords(*blockGlobalPos);
                    chunkRelPos = getChunkRelCoords(*blockGlobalPos);

                    // MAÑANA. NO SE ESTÁ RECALCULANDO EL IS OPAQUE CUANDO SE DESERIALIZA UN CHUNK GUARDADO
                    if (firstSecondLoopIteration || !ownedChunks[chunkPosOffset]->blockData().isOpaque.at(chunkRelPos)) {

                        recoverLightFromRemovedBlock_(colorChannel::RED, ownedChunks, *blockGlobalPos,
                            chunkPosOffset, chunkRelPos, blockPositions, blockLightsToRepropagate);
                        recoverLightFromRemovedBlock_(colorChannel::GREEN, ownedChunks, *blockGlobalPos,
                            chunkPosOffset, chunkRelPos, blockPositions, blockLightsToRepropagate);
                        recoverLightFromRemovedBlock_(colorChannel::BLUE, ownedChunks, *blockGlobalPos,
                            chunkPosOffset, chunkRelPos, blockPositions, blockLightsToRepropagate);
                        recoverLightFromRemovedBlock_(colorChannel::ALPHA, ownedChunks, *blockGlobalPos,
                            chunkPosOffset, chunkRelPos, blockPositions, blockLightsToRepropagate);

                    }
                    blockPositions.pop_front();
                    firstSecondLoopIteration = false;

                } while (blockPositions.size() > 0);

                // Repropagate the lights that have been affected by the removal but not have been eliminated.
                recalculateBlockLight_(c, priorityUpdate, &blockLightsToRepropagate);

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

    void chunkManager::removeLightFromPlacedBlock_(colorChannel channel, std::unordered_map<ivec3, chunk*>& ownedChunks, 
        const ivec3& blockGlobalPos, 
        const ivec3& chunkPosOffset, const ivec3& chunkRelPos, std::list<ivec3>& blockPositions,
        bool ignoreIntensityComparison, std::unordered_set<ivec3>& blockLightsToRepropagate, const block& placedBlock) {

        chunk* ownedChunk = ownedChunks[chunkPosOffset];
        if (ownedChunk->hasBlockLightInitialised()) {
        
            lightIntensity intensity = ownedChunk->blockData().blockLightColor->operator[](chunkRelPos).intensity(channel);

            if (intensity) {

                // chunkRelPos has a value that comes from assumming the chunk with offset 0 0 0 is the origin of the coordinates system.
                lightIntensity expectedIntensity = expectedIntensityFromNeighbors_(chunkPosOffset, chunkRelPos, channel, ownedChunks);

                // ALSO CAMBIAR intensity > expectedIntensity POR intensity != expectedIntensity???
                if (ignoreIntensityComparison || intensity > expectedIntensity) {

                    // Remove light.
                    lightIntensity intensityToSet = 0;
                    lightValue valueToSet = 0;
                    chunkUtility::setBlockLight(chunkRelPos, placedBlock.emittedLight(), channel, intensityToSet, valueToSet);
                    ownedChunk->setBlockLight(chunkRelPos, intensityToSet, valueToSet, channel);

                    // Search neighbors.
                    blockPositions.emplace_back(blockGlobalPos + ivec3FixedNorth);
                    blockPositions.emplace_back(blockGlobalPos + ivec3FixedSouth);
                    blockPositions.emplace_back(blockGlobalPos + ivec3FixedUp);
                    blockPositions.emplace_back(blockGlobalPos + ivec3FixedDown);
                    blockPositions.emplace_back(blockGlobalPos + ivec3FixedEast);
                    blockPositions.emplace_back(blockGlobalPos + ivec3FixedWest);

                    // Recalculate light blending across chunks.
                    ivec3 neighborOffset(
                        chunkRelPos.x >= CHUNK_SIZE_LIMIT ? 1 : chunkRelPos.x <= 0 ? -1 : 0,
                        chunkRelPos.y >= CHUNK_SIZE_LIMIT ? 1 : chunkRelPos.y <= 0 ? -1 : 0,
                        chunkRelPos.z >= CHUNK_SIZE_LIMIT ? 1 : chunkRelPos.z <= 0 ? -1 : 0
                    );
                    ivec3 neighborPos = vec3Zero;
                    ivec3 neighborRelPos = vec3Zero;
                    if (neighborOffset.x != 0) {

                        if (neighborOffset.y != 0) {

                            // X Y Z
                            if (neighborOffset.z != 0) {

                                neighborPos = chunkPosOffset + neighborOffset;
                                neighborRelPos = getChunkRelCoords(chunkRelPos + neighborOffset);

                                // This light values used for bending across chunk will be properly readded in the repropagation step.
                                ownedChunk->setBlockLight(chunkRelPos + neighborOffset, intensityToSet, valueToSet, channel);
                                ownedChunks[neighborPos]->setBlockLight(neighborRelPos - neighborOffset, intensityToSet, valueToSet, channel);

                            }

                            // X Y 0
                            neighborPos = ivec3(chunkPosOffset.x + neighborOffset.x, chunkPosOffset.y + neighborOffset.y, chunkPosOffset.z);
                            neighborRelPos =
                                getChunkRelCoords(ivec3(chunkRelPos.x + neighborOffset.x, chunkRelPos.y + neighborOffset.y, chunkRelPos.z));

                            ownedChunk->setBlockLight(chunkRelPos, neighborOffset.x, neighborOffset.y, 0, intensityToSet, valueToSet, channel);
                            ownedChunks[neighborPos]->setBlockLight(neighborRelPos, -neighborOffset.x, -neighborOffset.y, 0, intensityToSet, valueToSet, channel);

                        }

                        // X 0 Z
                        if (neighborOffset.z != 0) {

                            neighborPos = ivec3(chunkPosOffset.x + neighborOffset.x, chunkPosOffset.y, chunkPosOffset.z + neighborOffset.z);
                            neighborRelPos =
                                getChunkRelCoords(ivec3(chunkRelPos.x + neighborOffset.x, chunkRelPos.y, chunkRelPos.z + neighborOffset.z));

                            ownedChunk->setBlockLight(chunkRelPos, neighborOffset.x, 0, neighborOffset.z, intensityToSet, valueToSet, channel);
                            ownedChunks[neighborPos]->setBlockLight(neighborRelPos, -neighborOffset.x, 0, -neighborOffset.z, intensityToSet, valueToSet, channel);
                        }

                        // X 0 0
                        neighborPos = ivec3(chunkPosOffset.x + neighborOffset.x, chunkPosOffset.y, chunkPosOffset.z);
                        neighborRelPos = getChunkRelCoords(ivec3(chunkRelPos.x + neighborOffset.x, chunkRelPos.y, chunkRelPos.z));

                        ownedChunk->setBlockLight(chunkRelPos, neighborOffset.x, 0, 0, intensityToSet, valueToSet, channel);
                        ownedChunks[neighborPos]->setBlockLight(neighborRelPos, -neighborOffset.x, 0, 0, intensityToSet, valueToSet, channel);

                    }

                    if (neighborOffset.y != 0) {

                        // 0 Y Z
                        if (neighborOffset.z != 0) {

                            neighborPos = ivec3(chunkPosOffset.x, chunkPosOffset.y + neighborOffset.y, chunkPosOffset.z + neighborOffset.z);
                            neighborRelPos = getChunkRelCoords(ivec3(chunkRelPos.x, chunkRelPos.y + neighborOffset.y, chunkRelPos.z + neighborOffset.z));

                            ownedChunk->setBlockLight(chunkRelPos, 0, neighborOffset.y, neighborOffset.z, intensityToSet, valueToSet, channel);
                            ownedChunks[neighborPos]->setBlockLight(neighborRelPos, 0, -neighborOffset.y, -neighborOffset.z, intensityToSet, valueToSet, channel);

                        }

                        // 0 Y 0
                        neighborPos = ivec3(chunkPosOffset.x, chunkPosOffset.y + neighborOffset.y, chunkPosOffset.z);
                        neighborRelPos = getChunkRelCoords(ivec3(chunkRelPos.x, chunkRelPos.y + neighborOffset.y, chunkRelPos.z));

                        ownedChunk->setBlockLight(chunkRelPos, 0, neighborOffset.y, 0, intensityToSet, valueToSet, channel);
                        ownedChunks[neighborPos]->setBlockLight(neighborRelPos, 0, -neighborOffset.y, 0, intensityToSet, valueToSet, channel);

                    }

                    // 0 0 Z
                    if (neighborOffset.z != 0) {

                        neighborPos = ivec3(chunkPosOffset.x, chunkPosOffset.y, chunkPosOffset.z + neighborOffset.z);
                        neighborRelPos = getChunkRelCoords(ivec3(chunkRelPos.x, chunkRelPos.y, chunkRelPos.z + neighborOffset.z));

                        ownedChunk->setBlockLight(chunkRelPos, 0, 0, neighborOffset.z, intensityToSet, valueToSet, channel);
                        ownedChunks[neighborPos]->setBlockLight(neighborRelPos, 0, 0, -neighborOffset.z, intensityToSet, valueToSet, channel);

                    }

                }
                else
                    blockLightsToRepropagate.insert(blockGlobalPos);
            }
        
        }

    }

    // MAÑANA. REVISAR TODO TENIENDO EN CUENTA QUE SETBLOCKLIGHT Y GETBLOCKLIGHT TIENEN EN CUENTA EL CASO DE QUE LA ESTRUCTURA DE BLOCK LIGHT NO ESTÉ INICIALIZADA EN ESE MOMENTO.

    void chunkManager::recoverLightFromRemovedBlock_(colorChannel channel, std::unordered_map<ivec3, chunk*>& ownedChunks,
        const ivec3& blockGlobalPos,
        const ivec3& chunkPosOffset, const ivec3& chunkRelPos, std::list<ivec3>& blockPositions,
        std::unordered_set<ivec3>& blockLightsToRepropagate) {

        const blockLight& expectedBlockLight = expectedBlockLightFromNeighbors_(chunkPosOffset, chunkRelPos, channel, ownedChunks);
        lightIntensity expectedIntensity = expectedBlockLight.intensity(channel);
        lightValue expectedValue = expectedBlockLight.value(channel);

        if (expectedIntensity > 0) {
        
            chunk* ownedChunk = ownedChunks.at(chunkPosOffset);
            ownedChunk->initBlockLight();

            // chunkRelPos has a value that comes from assumming the chunk with offset 0 0 0 is the origin of the coordinates system.
            lightIntensity intensity = ownedChunk->blockData().blockLightColor->operator[](chunkRelPos).intensity(channel);
            std::vector<lightIntensity> neighborsIntensity(N_BLOCK_DIRECT_NEIGHBORS, 0);

            if (expectedIntensity > intensity) {

                // Remove light.
                ownedChunk->setBlockLight(chunkRelPos, expectedIntensity, expectedValue, channel);

                // Search neighbors.
                blockPositions.emplace_back(blockGlobalPos + ivec3FixedNorth);
                blockPositions.emplace_back(blockGlobalPos + ivec3FixedSouth);
                blockPositions.emplace_back(blockGlobalPos + ivec3FixedUp);
                blockPositions.emplace_back(blockGlobalPos + ivec3FixedDown);
                blockPositions.emplace_back(blockGlobalPos + ivec3FixedEast);
                blockPositions.emplace_back(blockGlobalPos + ivec3FixedWest);

                // Recalculate light blending across chunks.
                ivec3 neighborOffset(
                    chunkRelPos.x >= CHUNK_SIZE_LIMIT ? 1 : chunkRelPos.x <= 0 ? -1 : 0,
                    chunkRelPos.y >= CHUNK_SIZE_LIMIT ? 1 : chunkRelPos.y <= 0 ? -1 : 0,
                    chunkRelPos.z >= CHUNK_SIZE_LIMIT ? 1 : chunkRelPos.z <= 0 ? -1 : 0
                );
                chunk* neighborChunk = nullptr;
                ivec3 neighborPos = vec3Zero;
                ivec3 neighborRelPos = vec3Zero;
                if (neighborOffset.x != 0) {

                    if (neighborOffset.y != 0) {

                        // X Y Z
                        if (neighborOffset.z != 0) {

                            neighborPos = chunkPosOffset + neighborOffset;
                            neighborRelPos = getChunkRelCoords(chunkRelPos + neighborOffset);
                            neighborChunk = ownedChunks[neighborPos];

                            // This light values used for bending across chunk will be properly readded in the repropagation step.
                            ownedChunk->setBlockLight(chunkRelPos + neighborOffset,
                                neighborChunk->getBlockLight(neighborRelPos).intensity(channel),
                                expectedValue, channel);
                            neighborChunk->setBlockLight(neighborRelPos - neighborOffset, expectedIntensity, expectedValue, channel);

                        }

                        // X Y 0
                        neighborPos = ivec3(chunkPosOffset.x + neighborOffset.x, chunkPosOffset.y + neighborOffset.y, chunkPosOffset.z);
                        neighborRelPos =
                            getChunkRelCoords(ivec3(chunkRelPos.x + neighborOffset.x, chunkRelPos.y + neighborOffset.y, chunkRelPos.z));
                        neighborChunk = ownedChunks[neighborPos];

                        ownedChunk->setBlockLight(chunkRelPos, neighborOffset.x, neighborOffset.y, 0,
                            neighborChunk->getBlockLight(neighborRelPos).intensity(channel),
                            expectedValue, channel);
                        neighborChunk->setBlockLight(neighborRelPos, -neighborOffset.x, -neighborOffset.y, 0, expectedIntensity, expectedValue, channel);

                    }

                    // X 0 Z
                    if (neighborOffset.z != 0) {

                        neighborPos = ivec3(chunkPosOffset.x + neighborOffset.x, chunkPosOffset.y, chunkPosOffset.z + neighborOffset.z);
                        neighborRelPos =
                            getChunkRelCoords(ivec3(chunkRelPos.x + neighborOffset.x, chunkRelPos.y, chunkRelPos.z + neighborOffset.z));
                        neighborChunk = ownedChunks[neighborPos];

                        ownedChunk->setBlockLight(chunkRelPos, neighborOffset.x, 0, neighborOffset.z,
                            neighborChunk->getBlockLight(neighborRelPos).intensity(channel),
                            expectedValue, channel);
                        neighborChunk->setBlockLight(neighborRelPos, -neighborOffset.x, 0, -neighborOffset.z, expectedIntensity, expectedValue, channel);
                    }

                    // X 0 0
                    neighborPos = ivec3(chunkPosOffset.x + neighborOffset.x, chunkPosOffset.y, chunkPosOffset.z);
                    neighborRelPos = getChunkRelCoords(ivec3(chunkRelPos.x + neighborOffset.x, chunkRelPos.y, chunkRelPos.z));
                    neighborChunk = ownedChunks[neighborPos];

                    ownedChunk->setBlockLight(chunkRelPos, neighborOffset.x, 0, 0,
                        neighborChunk->getBlockLight(neighborRelPos).intensity(channel),
                        expectedValue, channel);
                    neighborChunk->setBlockLight(neighborRelPos, -neighborOffset.x, 0, 0, expectedIntensity, expectedValue, channel);

                }

                if (neighborOffset.y != 0) {

                    // 0 Y Z
                    if (neighborOffset.z != 0) {

                        neighborPos = ivec3(chunkPosOffset.x, chunkPosOffset.y + neighborOffset.y, chunkPosOffset.z + neighborOffset.z);
                        neighborRelPos = getChunkRelCoords(ivec3(chunkRelPos.x, chunkRelPos.y + neighborOffset.y, chunkRelPos.z + neighborOffset.z));
                        neighborChunk = ownedChunks[neighborPos];

                        ownedChunk->setBlockLight(chunkRelPos, 0, neighborOffset.y, neighborOffset.z,
                            neighborChunk->getBlockLight(neighborRelPos).intensity(channel),
                            expectedValue, channel);
                        neighborChunk->setBlockLight(neighborRelPos, 0, -neighborOffset.y, -neighborOffset.z, expectedIntensity, expectedValue, channel);

                    }

                    // 0 Y 0
                    neighborPos = ivec3(chunkPosOffset.x, chunkPosOffset.y + neighborOffset.y, chunkPosOffset.z);
                    neighborRelPos = getChunkRelCoords(ivec3(chunkRelPos.x, chunkRelPos.y + neighborOffset.y, chunkRelPos.z));
                    neighborChunk = ownedChunks[neighborPos];

                    ownedChunk->setBlockLight(chunkRelPos, 0, neighborOffset.y, 0,
                        neighborChunk->getBlockLight(neighborRelPos).intensity(channel),
                        expectedValue, channel);
                    neighborChunk->setBlockLight(neighborRelPos, 0, -neighborOffset.y, 0, expectedIntensity, expectedValue, channel);

                }

                // 0 0 Z
                if (neighborOffset.z != 0) {

                    neighborPos = ivec3(chunkPosOffset.x, chunkPosOffset.y, chunkPosOffset.z + neighborOffset.z);
                    neighborRelPos = getChunkRelCoords(ivec3(chunkRelPos.x, chunkRelPos.y, chunkRelPos.z + neighborOffset.z));
                    neighborChunk = ownedChunks[neighborPos];

                    ownedChunk->setBlockLight(chunkRelPos, 0, 0, neighborOffset.z,
                        neighborChunk->getBlockLight(neighborRelPos).intensity(channel),
                        expectedValue, channel);
                    neighborChunk->setBlockLight(neighborRelPos, 0, 0, -neighborOffset.z, expectedIntensity, expectedValue, channel);

                }

            }
        
        }

    }

    const blockLight& chunkManager::expectedBlockLightFromNeighbors_(const ivec3& chunkPosOffset, const ivec3& chunkRelPos, colorChannel channel,
        std::unordered_map<ivec3, chunk*>& ownedChunks) {

        Padded3DArray<blockLight>* blockLightPlusX =
            ownedChunks[chunkPosOffset + ivec3(chunkRelPos.x >= CHUNK_SIZE_LIMIT, 0, 0)]->blockData().blockLightColor;

        Padded3DArray<blockLight>* blockLightMinusX =
            ownedChunks[chunkPosOffset + ivec3(-(chunkRelPos.x <= 0), 0, 0)]->blockData().blockLightColor;

        Padded3DArray<blockLight>* blockLightPlusY =
            ownedChunks[chunkPosOffset + ivec3(0, chunkRelPos.y >= CHUNK_SIZE_LIMIT, 0)]->blockData().blockLightColor;

        Padded3DArray<blockLight>* blockLightMinusY =
            ownedChunks[chunkPosOffset + ivec3(0, -(chunkRelPos.y <= 0), 0)]->blockData().blockLightColor;

        Padded3DArray<blockLight>* blockLightPlusZ =
            ownedChunks[chunkPosOffset + ivec3(0, 0, chunkRelPos.z >= CHUNK_SIZE_LIMIT)]->blockData().blockLightColor;

        Padded3DArray<blockLight>* blockLightMinusZ =
            ownedChunks[chunkPosOffset + ivec3(0, 0, -(chunkRelPos.z <= 0))]->blockData().blockLightColor;

        const blockLight* lightPlusX = blockLightPlusX ? 
            &blockLightPlusX->get(getChunkRelCoords(chunkRelPos + ivec3FixedNorth)) : &blockLight::zero();

        const blockLight* lightMinusX = blockLightMinusX ?
            &blockLightMinusX->get(getChunkRelCoords(chunkRelPos + ivec3FixedSouth)) : &blockLight::zero();

        const blockLight* lightPlusY = blockLightPlusY ?
            &blockLightPlusY->get(getChunkRelCoords(chunkRelPos + ivec3FixedUp)) : &blockLight::zero();

        const blockLight* lightMinusY = blockLightMinusY ?
            &blockLightMinusY->get(getChunkRelCoords(chunkRelPos + ivec3FixedDown)) : &blockLight::zero();

        const blockLight* lightPlusZ = blockLightPlusZ ? 
            &blockLightPlusZ->get(getChunkRelCoords(chunkRelPos + ivec3FixedEast)) : &blockLight::zero();

        const blockLight* lightMinusZ = blockLightMinusZ ?
            &blockLightMinusZ->get(getChunkRelCoords(chunkRelPos + ivec3FixedWest)) : &blockLight::zero();

        const blockLight* selectedLight =
            lightPlusX->intensity(channel) > lightMinusX->intensity(channel) ? lightPlusX : lightMinusX;
        selectedLight =
            selectedLight->intensity(channel) > lightPlusY->intensity(channel) ? selectedLight : lightPlusY;
        selectedLight =
            selectedLight->intensity(channel) > lightMinusY->intensity(channel) ? selectedLight : lightMinusY;
        selectedLight =
            selectedLight->intensity(channel) > lightPlusZ->intensity(channel) ? selectedLight : lightPlusZ;
        selectedLight =
            selectedLight->intensity(channel) > lightMinusZ->intensity(channel) ? selectedLight : lightMinusZ;

        return selectedLight->decreased(channel);
    }

    const blockLight& chunkManager::expectedBlockLightFromNeighbors_(const ivec3& chunkPosOffset, const ivec3& chunkRelPos, colorChannel channel,
        std::unordered_map<ivec3, chunk*>& ownedChunks, std::vector<lightIntensity>* neighborsIntensity) {

        const blockLight* lightPlusX =
            &ownedChunks[chunkPosOffset + ivec3(chunkRelPos.x >= CHUNK_SIZE_LIMIT, 0, 0)]->blockData()
            .blockLightColor->get(getChunkRelCoords(chunkRelPos + ivec3FixedNorth));

        const blockLight* lightMinusX =
            &ownedChunks[chunkPosOffset + ivec3(-(chunkRelPos.x <= 0), 0, 0)]->blockData()
            .blockLightColor->get(getChunkRelCoords(chunkRelPos + ivec3FixedSouth));

        const blockLight* lightPlusY =
            &ownedChunks[chunkPosOffset + ivec3(0, chunkRelPos.y >= CHUNK_SIZE_LIMIT, 0)]->blockData()
            .blockLightColor->get(getChunkRelCoords(chunkRelPos + ivec3FixedUp));

        const blockLight* lightMinusY =
            &ownedChunks[chunkPosOffset + ivec3(0, -(chunkRelPos.y <= 0), 0)]->blockData()
            .blockLightColor->get(getChunkRelCoords(chunkRelPos + ivec3FixedDown));

        const blockLight* lightPlusZ =
            &ownedChunks[chunkPosOffset + ivec3(0, 0, chunkRelPos.z >= CHUNK_SIZE_LIMIT)]->blockData()
            .blockLightColor->get(getChunkRelCoords(chunkRelPos + ivec3FixedEast));

        const blockLight* lightMinusZ =
            &ownedChunks[chunkPosOffset + ivec3(0, 0, -(chunkRelPos.z <= 0))]->blockData()
            .blockLightColor->get(getChunkRelCoords(chunkRelPos + ivec3FixedWest));

        const blockLight* selectedLight = 
            lightPlusX->intensity(channel) > lightMinusX->intensity(channel) ? lightPlusX : lightMinusX;
        selectedLight = 
            selectedLight->intensity(channel) > lightPlusY->intensity(channel) ? selectedLight : lightPlusY;
        selectedLight =
            selectedLight->intensity(channel) > lightMinusY->intensity(channel) ? selectedLight : lightMinusY;
        selectedLight =
            selectedLight->intensity(channel) > lightPlusZ->intensity(channel) ? selectedLight : lightPlusZ;
        selectedLight =
            selectedLight->intensity(channel) > lightMinusZ->intensity(channel) ? selectedLight : lightMinusZ;

        if (neighborsIntensity) {
        
            if (neighborsIntensity->size() != N_BLOCK_DIRECT_NEIGHBORS)
                logger::errorLog("There can only be " + std::to_string(N_BLOCK_DIRECT_NEIGHBORS) + " direct block neighbors");
            else {
            
                neighborsIntensity->operator[](0) = lightPlusX->intensity(channel);
                neighborsIntensity->operator[](1) = lightMinusX->intensity(channel);
                neighborsIntensity->operator[](2) = lightPlusY->intensity(channel);
                neighborsIntensity->operator[](3) = lightMinusY->intensity(channel);
                neighborsIntensity->operator[](4) = lightPlusZ->intensity(channel);
                neighborsIntensity->operator[](5) = lightMinusZ->intensity(channel);
            
            }
        
        }
        return selectedLight->decreased(channel);
    }

    bool chunkManager::pushChunkTask_(chunkJobType type, void* data, job& j) {
    
        bool shouldBeProcessed = true;
        chunk* c = nullptr;

        // Set the task.
        switch (type) {

        case chunkJobType::NONE:
            logger::errorLog("No chunk job type was specified");
            break;
        case chunkJobType::LOAD:
            j.pushBackTask(loadChunkJob, static_cast<chunk*>(data));
            break;
        case chunkJobType::LOAD2:
            c = static_cast<chunk*>(data);
            c->loadStatusMutex().lock();
            if (c->loadStatus() == chunkLoadStatus::BASICTERRAIN || c->loadStatus() == chunkLoadStatus::BASICTERRAINFROMDISK) {
                c->loadStatus(chunkLoadStatus::PENDING_DECORATED);
                j.pushBackTask(loadChunkJobPass2, c);
            }
            else
                shouldBeProcessed = false;
            c->loadStatusMutex().unlock();
            break;
        case chunkJobType::ONLYREMESH:
            // MAÑANA. METER LOS CHUNK JOBS O EN UN NAMESPACE DENTRO DE LA CLASE O DE ALGUNA MANERA SIMILAR PARA PODER
            // QUITARLES EL SUFIJO DE CHUNKJOB. 
            j.pushBackTask(remeshChunkJob, static_cast<chunk*>(data));
            break;
        case chunkJobType::UNLOADANDSAVE:
            j.pushBackTask(unloadAndSaveChunkJob, static_cast<chunk*>(data));
            break;
        case chunkJobType::PRIORITYREMESH:
            j.pushBackTask(priorityRemeshChunkJob, static_cast<chunk*>(data));
            break;
        case chunkJobType::PRIORITYREMESH_ADDEDLIGHT:
            j.pushBackTask(priorityRemeshAddedLightChunkJob, static_cast<chunk*>(data));
            break;
        case chunkJobType::PRIORITYREMESH_REMOVEDLIGHT:
            j.pushBackTask(priorityRemeshRemovedLightChunkJob, data);
            break;
        case chunkJobType::NON_TRANSPARENT_BLOCK_PLACED_ON_LIGHT:
            j.pushBackTask(nonTransparentBlockPlacedOnLightChunkJob, data);
            break;
        case chunkJobType::NON_TRANSPARENT_BLOCK_REMOVED_ON_LIGHT:
            j.pushBackTask(nonTransparentBlockRemovedOnLightChunkJob, data);
            break;
        default:
            logger::errorLog("Unsupported chunkJobType type " + std::to_string(static_cast<int>(type)));
            break;
        }

        return shouldBeProcessed;
    
    }

    void chunkManager::sendJob_(job& j, bool pushJobBack, bool isPriority) {
    
        if(isPriority)
            priorityChunkTasks_->submitJob(j, pushJobBack);
        else
            chunkTasks_->submitJob(j, pushJobBack);
    
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

            //c->poraqui16 = true;
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
        //c->poraqui24 = true;
        chunksPool_.free(*c);

    }

    void chunkManager::priorityRemeshChunkJob(void* data) {

        chunk* c = static_cast<chunk*>(data);
        c->needsRemesh(true);
        chunkManager::remesh(c, true);

    }

    void chunkManager::priorityRemeshAddedLightChunkJob(void* data) {

        chunk* c = static_cast<chunk*>(data);
        c->needsRemesh(true);
        c->getAndOwnChunks();
        recalculateBlockLight_(*c, true);
        c->disownChunks();

    }

    void chunkManager::priorityRemeshRemovedLightChunkJob(void* rawData) {

        std::tuple<chunk*, std::list<ivec3>*>* data = static_cast<std::tuple<chunk*, std::list<ivec3>*>*>(rawData);
        chunk* c = std::get<chunk*>(*data);
        std::list<ivec3>* lightsToRemove = std::get<std::list<ivec3>*>(*data);
        c->needsRemesh(true);
        c->getAndOwnChunks();
        recalculateBlockLightAfterRemoval_(*c, true, *lightsToRemove);
        c->disownChunks();

        // Delete the objects that were created just for this operation.
        delete lightsToRemove;
        delete data;

    }

    void chunkManager::nonTransparentBlockPlacedOnLightChunkJob(void* rawData) {
    
        std::tuple<chunk*, std::list<ivec3>*, const block*>* data = static_cast<std::tuple<chunk*, std::list<ivec3>*, const block*>*>(rawData);
        chunk* c = std::get<chunk*>(*data);
        std::list<ivec3>* nonTransparentPlaced = std::get<std::list<ivec3>*>(*data);
        const block* placedBlock = std::get<const block*>(*data);
        c->needsRemesh(true);
        c->getAndOwnChunks(); // MAÑANA. FACTORIZE THIS SO THAT THIS CAN ONLY BE DONE ONCE FOR THE ENTIRE JOB, NOT FOR EACH TASK.
        recalculateBlockLightAfterNonTransparentPlaced_(*c, true, *nonTransparentPlaced, *placedBlock);
        c->disownChunks();

        // Delete the objects that were created just for this operation.
        delete nonTransparentPlaced;
        delete data;
    
    }

    void chunkManager::nonTransparentBlockRemovedOnLightChunkJob(void* rawData) {

        std::tuple<chunk*, std::list<ivec3>*>* data = static_cast<std::tuple<chunk*, std::list<ivec3>*>*>(rawData);
        chunk* c = std::get<chunk*>(*data);
        std::list<ivec3>* nonTransparentPlaced = std::get<std::list<ivec3>*>(*data);
        c->needsRemesh(true);
        c->getAndOwnChunks(); // MAÑANA. FACTORIZE THIS SO THAT THIS CAN ONLY BE DONE ONCE FOR THE ENTIRE JOB, NOT FOR EACH TASK.
        recalculateBlockLightAfterNonTransparentRemoved_(*c, true, *nonTransparentPlaced);
        c->disownChunks();

        // Delete the objects that were created just for this operation.
        delete nonTransparentPlaced;
        delete data;

    }

    void chunkManager::processLightJob(void* rawData) {

        std::tuple<chunk*, bool>* data = static_cast<std::tuple<chunk*, bool>*>(rawData);
        chunk* c = std::get<chunk*>(*data);
        bool causesPriorityUpdate = std::get<bool>(*data);

        c->loadStatusMutex().lock();
        if (c->loadStatus() == chunkLoadStatus::PENDING_LIGHTS_APPLIED) {
            c->loadStatusMutex().unlock();
        
            recalculateBlockLight_(*c, causesPriorityUpdate);

            // Delete the objects that were created just for this operation.
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
