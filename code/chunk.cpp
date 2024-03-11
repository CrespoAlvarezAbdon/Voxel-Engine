#include "chunk.h"
#include <algorithm>
#include <ctime>
#include <cstdlib>
#include <cstring>
#include <cmath>
#include <chrono>
#include <cstring>
#include <fstream>
#include <format>
#include <filesystem>
#include <iostream>
#include "AIAPI.h"
#include "camera.h"
#include "player.h"
#include "input.h"
#include "gui.h"
#include "game.h"
#include "graphics.h"
#include "logger.h"
#include "timer.h"
#include "AI/AIGameEx1.h"


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
      loadLevel_(chunkStatus::NOTLOADED) {
    
        renderingData_.chunkPos = vec3Zero;

        std::memset(blocksLocalIDs, 0, nBlocksChunk * sizeof(unsigned short));

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
      loadLevel_(chunkStatus::NOTLOADED)
    {

        renderingData_.chunkPos = chunkPos;

        std::memset(blocksLocalIDs, 0, nBlocksChunk * sizeof(unsigned short));

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
      loadLevel_(c.loadLevel_.load()){

        std::unique_lock<std::shared_mutex> lock(blocksMutex_);

        palette_ = c.palette_;
        paletteCount_ = c.paletteCount_;
        freeLocalIDs_ = c.freeLocalIDs_;

        std::memcpy(blocksLocalIDs, c.blocksLocalIDs, nBlocksChunk * sizeof(unsigned short));

        std::memcpy(neighborBlocksPlusX_, c.neighborBlocksPlusX_, nBlocksChunkEdge * sizeof(unsigned short));
        std::memcpy(neighborBlocksMinusX_, c.neighborBlocksMinusX_, nBlocksChunkEdge * sizeof(unsigned short));
        std::memcpy(neighborBlocksPlusY_, c.neighborBlocksPlusY_, nBlocksChunkEdge * sizeof(unsigned short));
        std::memcpy(neighborBlocksMinusY_, c.neighborBlocksMinusY_, nBlocksChunkEdge * sizeof(unsigned short));
        std::memcpy(neighborBlocksPlusZ_, c.neighborBlocksPlusZ_, nBlocksChunkEdge * sizeof(unsigned short));
        std::memcpy(neighborBlocksMinusZ_, c.neighborBlocksMinusZ_, nBlocksChunkEdge * sizeof(unsigned short));

        renderingData_.chunkPos = c.renderingData_.chunkPos;
        renderingData_.vertices = c.renderingData_.vertices;

    }

    const block& chunk::getBlock(GLbyte x, GLbyte y, GLbyte z) {

        std::shared_lock<std::shared_mutex> lock(blocksMutex_);

        unsigned int localID = blocksLocalIDs[x][y][z];

        return localID ? block::getBlockC(palette_.getT2(localID)) : block::emptyBlock();

    }

    const block& chunk::setBlock(GLbyte x, GLbyte y, GLbyte z, const block& b, bool modification) {

        unsigned short& actualLocalID = blocksLocalIDs[x][y][z];
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

        return block::getBlockC(oldGlobalID);

    }

    // TODO. MOVER AL FINAL DE CHUNK.
    void chunk::placeNewBlock(unsigned short& actualLocalID, const block& newBlock) {

        unsigned int newGlobalID = newBlock.intID(),
                     oldGlobalID = actualLocalID ? palette_.getT2(actualLocalID) : 0;
        needsRemesh_ = needsRemesh_ || oldGlobalID != newGlobalID;

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

    void chunk::setBlockNeighbor(unsigned int firstIndex, unsigned int secondIndex, blockViewDir neighbor, const block& block, bool modification) {

        unsigned short oldLocalID = 0;
        bool blockWasModified = false;
        
        if (neighbor == blockViewDir::NONE)
            logger::errorLog("No block view direction was specified");
        else if (neighbor == blockViewDir::PLUSX) {

            unsigned short& actualLocalID = neighborBlocksPlusX_[firstIndex][secondIndex];
            oldLocalID = actualLocalID;
            placeNewBlock(actualLocalID, block);
            blockWasModified = oldLocalID != actualLocalID;
            needsRemesh_ = needsRemesh_ || blockWasModified && blocksLocalIDs[VoxelEng::SCX - 1][firstIndex][secondIndex];
            if (!oldLocalID && actualLocalID)
                nBlocksPlusX_++;
            else if (oldLocalID && !actualLocalID)
                nBlocksPlusX_--;

        }
        else if (neighbor == blockViewDir::NEGX) {

            unsigned short& actualLocalID = neighborBlocksMinusX_[firstIndex][secondIndex];
            oldLocalID = actualLocalID;
            placeNewBlock(actualLocalID, block);
            blockWasModified = oldLocalID != actualLocalID;
            needsRemesh_ = needsRemesh_ || blockWasModified && blocksLocalIDs[0][firstIndex][secondIndex];
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
            needsRemesh_ = needsRemesh_ || blockWasModified && blocksLocalIDs[firstIndex][VoxelEng::SCX - 1][secondIndex];
            if (!oldLocalID && actualLocalID)
                nBlocksPlusY_++;
            else if (oldLocalID && !actualLocalID)
                nBlocksPlusY_--;

        }
        else if (neighbor == blockViewDir::NEGY) {

            unsigned short& actualLocalID = neighborBlocksMinusY_[firstIndex][secondIndex];
            oldLocalID = actualLocalID;
            placeNewBlock(actualLocalID, block);
            blockWasModified = oldLocalID != actualLocalID;
            needsRemesh_ = needsRemesh_ || blockWasModified && blocksLocalIDs[firstIndex][0][secondIndex];
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
            needsRemesh_ = needsRemesh_ || blockWasModified && blocksLocalIDs[firstIndex][secondIndex][VoxelEng::SCX - 1];
            if (!oldLocalID && actualLocalID)
                nBlocksPlusZ_++;
            else if (oldLocalID && !actualLocalID)
                nBlocksPlusZ_--;

        }
        else if (neighbor == blockViewDir::NEGZ) {

            unsigned short& actualLocalID = neighborBlocksMinusZ_[firstIndex][secondIndex];
            oldLocalID = actualLocalID;
            placeNewBlock(actualLocalID, block);
            blockWasModified = oldLocalID != actualLocalID;
            needsRemesh_ = needsRemesh_ || blockWasModified && blocksLocalIDs[firstIndex][secondIndex][0];
            if (!oldLocalID && actualLocalID)
                nBlocksMinusZ_++;
            else if (oldLocalID && !actualLocalID)
                nBlocksMinusZ_--;

        }
        else
            logger::errorLog("Unsupported block view direction specified");

        modified_ = modified_ || (blockWasModified && modification);

    }

    bool chunk::renewMesh() {

        std::unique_lock<std::shared_mutex> lock(renderingDataMutex_);

        if (needsRemesh_) {

            needsRemesh_ = false;

            renderingData_.vertices = model();

            // Read chunk data section starts.
            blocksMutex_.lock_shared();

            // Render faces that do not require data from neighbor chunks.
            vertex aux;
            int x,
                y,
                z;
            const block* bNeighbor = nullptr;
            unsigned short neighborLocalID = 0;
            // Do not iterate through this if there are no blocks to render faces that belong to this chunk.
            if (nBlocks_ && nBlocks_ < nBlocksChunk) 
                for (x = 0; x < SCX; x++)
                    for (y = 0; y < SCY; y++)
                        for (z = 0; z < SCZ; z++) {

                            // Add block's model to the mesh if necessary.
                            if (!blocksLocalIDs[x][y][z]) {

                                // Front face vertices with culling of non-visible faces. z+
                                if (z < SCZLimit && (neighborLocalID = blocksLocalIDs[x][y][z + 1])) {

                                    bNeighbor = &block::getBlockC(palette_.getT2(neighborLocalID));

                                    // Create the face's vertices for face z-.
                                    for (int vertex = 0; vertex < blockTriangles_->operator[](0).size(); vertex++) {

                                        aux.positions[0] = renderingData_.chunkPos.x * SCX + x + blockVertices_->operator[](blockTriangles_->operator[](0)[vertex]).positions[0];
                                        aux.positions[1] = renderingData_.chunkPos.y * SCY + y + blockVertices_->operator[](blockTriangles_->operator[](0)[vertex]).positions[1];
                                        aux.positions[2] = renderingData_.chunkPos.z * SCZ + z + 1 + blockVertices_->operator[](blockTriangles_->operator[](0)[vertex]).positions[2];

                                        aux.normals = 0 | (0 << 30);
                                        aux.normals = aux.normals | (0 << 20);
                                        aux.normals = aux.normals | (512 << 10);
                                        aux.normals = aux.normals | (512 << 0);

                                        renderingData_.vertices.push_back(aux);

                                    }

                                    // Add texture to the face.
                                    models::addTexture(*bNeighbor, renderingData_.vertices);

                                }

                                // Back face vertices with culling of non-visible faces. z-
                                if (z > 0 && (neighborLocalID = blocksLocalIDs[x][y][z - 1])) {

                                    bNeighbor = &block::getBlockC(palette_.getT2(neighborLocalID));

                                    // Create the face's vertices for z-.
                                    for (int vertex = 0; vertex < blockTriangles_->operator[](0).size(); vertex++) {

                                        aux.positions[0] = renderingData_.chunkPos.x * SCX + x + blockVertices_->operator[](blockTriangles_->operator[](1)[vertex]).positions[0];
                                        aux.positions[1] = renderingData_.chunkPos.y * SCY + y + blockVertices_->operator[](blockTriangles_->operator[](1)[vertex]).positions[1];
                                        aux.positions[2] = renderingData_.chunkPos.z * SCZ + z - 1 + blockVertices_->operator[](blockTriangles_->operator[](1)[vertex]).positions[2];

                                        aux.normals = 0 | (0 << 30);
                                        aux.normals = aux.normals | (1023 << 20);
                                        aux.normals = aux.normals | (512 << 10);
                                        aux.normals = aux.normals | (512 << 0);

                                        renderingData_.vertices.push_back(aux);

                                    }

                                    // Add texture to the face.
                                    models::addTexture(*bNeighbor, renderingData_.vertices);

                                }
                                
                                // Top face vertices with culling of non-visible faces. y+
                                if (y < SCYLimit && (neighborLocalID = blocksLocalIDs[x][y + 1][z])) {

                                    bNeighbor = &block::getBlockC(palette_.getT2(neighborLocalID));

                                    // Create the face's vertices for face y-.
                                    for (int vertex = 0; vertex < blockTriangles_->operator[](0).size(); vertex++) {

                                        aux.positions[0] = renderingData_.chunkPos.x * SCX + x + blockVertices_->operator[](blockTriangles_->operator[](3)[vertex]).positions[0];
                                        aux.positions[1] = renderingData_.chunkPos.y * SCY + y + 1 + blockVertices_->operator[](blockTriangles_->operator[](3)[vertex]).positions[1];
                                        aux.positions[2] = renderingData_.chunkPos.z * SCZ + z + blockVertices_->operator[](blockTriangles_->operator[](3)[vertex]).positions[2];

                                        aux.normals = 0 | (0 << 30);
                                        aux.normals = aux.normals | (512 << 20);
                                        aux.normals = aux.normals | (0 << 10);
                                        aux.normals = aux.normals | (512 << 0);

                                        renderingData_.vertices.push_back(aux);

                                    }

                                    // Add texture to the face.
                                    models::addTexture(*bNeighbor, renderingData_.vertices);

                                }
                                
                                // Bottom face vertices with culling of non-visible faces. y-
                                if (y > 0 && (neighborLocalID = blocksLocalIDs[x][y - 1][z])) {

                                    bNeighbor = &block::getBlockC(palette_.getT2(neighborLocalID));

                                    // Create the face's vertices for face y+.
                                    for (int vertex = 0; vertex < blockTriangles_->operator[](0).size(); vertex++) {

                                        aux.positions[0] = renderingData_.chunkPos.x * SCX + x + blockVertices_->operator[](blockTriangles_->operator[](2)[vertex]).positions[0];
                                        aux.positions[1] = renderingData_.chunkPos.y * SCY + y - 1 + blockVertices_->operator[](blockTriangles_->operator[](2)[vertex]).positions[1];
                                        aux.positions[2] = renderingData_.chunkPos.z * SCZ + z + blockVertices_->operator[](blockTriangles_->operator[](2)[vertex]).positions[2];

                                        aux.normals = 0 | (0 << 30);
                                        aux.normals = aux.normals | (512 << 20);
                                        aux.normals = aux.normals | (1023 << 10);
                                        aux.normals = aux.normals | (512 << 0);

                                        renderingData_.vertices.push_back(aux);

                                    }

                                    // Add texture to the face.
                                    models::addTexture(*bNeighbor, renderingData_.vertices);

                                }
                                
                                // Right face vertices with culling of non-visible faces. x+
                                if (x < SCXLimit && (neighborLocalID = blocksLocalIDs[x + 1][y][z])) {

                                    bNeighbor = &block::getBlockC(palette_.getT2(neighborLocalID));

                                    // Create the face's vertices for face x-.
                                    for (int vertex = 0; vertex < blockTriangles_->operator[](0).size(); vertex++) {

                                        aux.positions[0] = renderingData_.chunkPos.x * SCX + x + 1 + blockVertices_->operator[](blockTriangles_->operator[](4)[vertex]).positions[0];
                                        aux.positions[1] = renderingData_.chunkPos.y * SCY + y + blockVertices_->operator[](blockTriangles_->operator[](4)[vertex]).positions[1];
                                        aux.positions[2] = renderingData_.chunkPos.z * SCZ + z + blockVertices_->operator[](blockTriangles_->operator[](4)[vertex]).positions[2];

                                        aux.normals = 0 | (0 << 30);
                                        aux.normals = aux.normals | (512 << 20);
                                        aux.normals = aux.normals | (512 << 10);
                                        aux.normals = aux.normals | (0 << 0);

                                        renderingData_.vertices.push_back(aux);

                                    }

                                    // Add texture to the face.
                                    models::addTexture(*bNeighbor, renderingData_.vertices);

                                }
                                
                                // Left face vertices with culling of non-visible faces. x-
                                if (x > 0 && (neighborLocalID = blocksLocalIDs[x - 1][y][z])) {

                                    bNeighbor = &block::getBlockC(palette_.getT2(neighborLocalID));

                                    // Create the face's vertices for face x+.
                                    for (int vertex = 0; vertex < blockTriangles_->operator[](0).size(); vertex++) {

                                        aux.positions[0] = renderingData_.chunkPos.x * SCX + x - 1 + blockVertices_->operator[](blockTriangles_->operator[](5)[vertex]).positions[0];
                                        aux.positions[1] = renderingData_.chunkPos.y * SCY + y + blockVertices_->operator[](blockTriangles_->operator[](5)[vertex]).positions[1];
                                        aux.positions[2] = renderingData_.chunkPos.z * SCZ + z + blockVertices_->operator[](blockTriangles_->operator[](5)[vertex]).positions[2];

                                        aux.normals = 0 | (0 << 30);
                                        aux.normals = aux.normals | (512 << 20);
                                        aux.normals = aux.normals | (512 << 10);
                                        aux.normals = aux.normals | (1023 << 0);

                                        renderingData_.vertices.push_back(aux);

                                    }

                                    // Add texture to the face.
                                    models::addTexture(*bNeighbor, renderingData_.vertices);

                                }
                                
                            }

                        }

            if (nBlocksPlusZ_) {
            
                for (x = 0; x < SCX; x++)
                    for (y = 0; y < SCY; y++) {
                    
                        // Add block's model to the mesh if necessary.
                        if (!blocksLocalIDs[x][y][SCZLimit]) {

                            // Front face vertices with culling of non-visible faces. z+
                            if (neighborLocalID = neighborBlocksPlusZ_[x][y]) {

                                bNeighbor = &block::getBlockC(palette_.getT2(neighborLocalID));

                                // Create the face's vertices for face z-.
                                for (int vertex = 0; vertex < blockTriangles_->operator[](0).size(); vertex++) {

                                    aux.positions[0] = renderingData_.chunkPos.x * SCX + x + blockVertices_->operator[](blockTriangles_->operator[](0)[vertex]).positions[0];
                                    aux.positions[1] = renderingData_.chunkPos.y * SCY + y + blockVertices_->operator[](blockTriangles_->operator[](0)[vertex]).positions[1];
                                    aux.positions[2] = (renderingData_.chunkPos.z + 1) * SCZ + blockVertices_->operator[](blockTriangles_->operator[](0)[vertex]).positions[2];

                                    aux.normals = 0 | (0 << 30);
                                    aux.normals = aux.normals | (0 << 20);
                                    aux.normals = aux.normals | (512 << 10);
                                    aux.normals = aux.normals | (512 << 0);

                                    renderingData_.vertices.push_back(aux);

                                }

                                // Add texture to the face.
                                models::addTexture(*bNeighbor, renderingData_.vertices);

                            }

                        }
                    
                    }
            
            }

            if (nBlocksMinusZ_) {

                for (x = 0; x < SCX; x++)
                    for (y = 0; y < SCY; y++) {

                        // Add block's model to the mesh if necessary.
                        if (!blocksLocalIDs[x][y][0]) {

                            if (neighborLocalID = neighborBlocksMinusZ_[x][y]) {

                                bNeighbor = &block::getBlockC(palette_.getT2(neighborLocalID));

                                // Create the face's vertices for z-.
                                for (int vertex = 0; vertex < blockTriangles_->operator[](0).size(); vertex++) {

                                    aux.positions[0] = renderingData_.chunkPos.x * SCX + x + blockVertices_->operator[](blockTriangles_->operator[](1)[vertex]).positions[0];
                                    aux.positions[1] = renderingData_.chunkPos.y * SCY + y + blockVertices_->operator[](blockTriangles_->operator[](1)[vertex]).positions[1];
                                    aux.positions[2] = (renderingData_.chunkPos.z - 1) * SCZ + 15 + blockVertices_->operator[](blockTriangles_->operator[](1)[vertex]).positions[2];

                                    aux.normals = 0 | (0 << 30);
                                    aux.normals = aux.normals | (1023 << 20);
                                    aux.normals = aux.normals | (512 << 10);
                                    aux.normals = aux.normals | (512 << 0);

                                    renderingData_.vertices.push_back(aux);

                                }

                                // Add texture to the face.
                                models::addTexture(*bNeighbor, renderingData_.vertices);

                            }

                        }

                    }

            }

            if (nBlocksPlusY_) {

                for (x = 0; x < SCX; x++)
                    for (z = 0; z < SCZ; z++) {

                        // Add block's model to the mesh if necessary.
                        if (!blocksLocalIDs[x][SCYLimit][z]) {

                            if (neighborLocalID = neighborBlocksPlusY_[x][z]) {

                                bNeighbor = &block::getBlockC(palette_.getT2(neighborLocalID));

                                // Create the face's vertices for face y+.
                                for (int vertex = 0; vertex < blockTriangles_->operator[](0).size(); vertex++) {

                                    aux.positions[0] = renderingData_.chunkPos.x * SCX + x + blockVertices_->operator[](blockTriangles_->operator[](3)[vertex]).positions[0];
                                    aux.positions[1] = (renderingData_.chunkPos.y + 1) * SCY + blockVertices_->operator[](blockTriangles_->operator[](3)[vertex]).positions[1];
                                    aux.positions[2] = renderingData_.chunkPos.z * SCZ + z + blockVertices_->operator[](blockTriangles_->operator[](3)[vertex]).positions[2];

                                    aux.normals = 0 | (0 << 30);
                                    aux.normals = aux.normals | (512 << 20);
                                    aux.normals = aux.normals | (0 << 10);
                                    aux.normals = aux.normals | (512 << 0);

                                    renderingData_.vertices.push_back(aux);

                                }

                                // Add texture to the face.
                                models::addTexture(*bNeighbor, renderingData_.vertices);

                            }

                        }

                    }

            }

            if (nBlocksMinusY_) {

                for (x = 0; x < SCX; x++)
                    for (z = 0; z < SCZ; z++) {

                        // Add block's model to the mesh if necessary.
                        if (!blocksLocalIDs[x][0][z]) {

                            if (neighborLocalID = neighborBlocksMinusY_[x][z]) {

                                bNeighbor = &block::getBlockC(palette_.getT2(neighborLocalID));

                                // Create the face's vertices for face y+.
                                for (int vertex = 0; vertex < blockTriangles_->operator[](0).size(); vertex++) {

                                    aux.positions[0] = renderingData_.chunkPos.x * SCX + x + blockVertices_->operator[](blockTriangles_->operator[](2)[vertex]).positions[0];
                                    aux.positions[1] = (renderingData_.chunkPos.y - 1) * SCY + 15 + blockVertices_->operator[](blockTriangles_->operator[](2)[vertex]).positions[1];
                                    aux.positions[2] = renderingData_.chunkPos.z * SCZ + z + blockVertices_->operator[](blockTriangles_->operator[](2)[vertex]).positions[2];

                                    aux.normals = 0 | (0 << 30);
                                    aux.normals = aux.normals | (512 << 20);
                                    aux.normals = aux.normals | (1023 << 10);
                                    aux.normals = aux.normals | (512 << 0);

                                    renderingData_.vertices.push_back(aux);

                                }

                                // Add texture to the face.
                                models::addTexture(*bNeighbor, renderingData_.vertices);

                            }

                        }

                    }

            }

            if (nBlocksPlusX_) {

                for (y = 0; y < SCY; y++)
                    for (z = 0; z < SCZ; z++) {

                        // Add block's model to the mesh if necessary.
                        if (!blocksLocalIDs[SCXLimit][y][z]) {

                            if (neighborLocalID = neighborBlocksPlusX_[y][z]) {

                                bNeighbor = &block::getBlockC(palette_.getT2(neighborLocalID));

                                // Create the face's vertices for face x+.
                                for (int vertex = 0; vertex < blockTriangles_->operator[](0).size(); vertex++) {

                                    aux.positions[0] = (renderingData_.chunkPos.x + 1) * SCX + blockVertices_->operator[](blockTriangles_->operator[](4)[vertex]).positions[0];
                                    aux.positions[1] = renderingData_.chunkPos.y * SCY + y + blockVertices_->operator[](blockTriangles_->operator[](4)[vertex]).positions[1];
                                    aux.positions[2] = renderingData_.chunkPos.z * SCZ + z + blockVertices_->operator[](blockTriangles_->operator[](4)[vertex]).positions[2];

                                    aux.normals = 0 | (0 << 30);
                                    aux.normals = aux.normals | (512 << 20);
                                    aux.normals = aux.normals | (512 << 10);
                                    aux.normals = aux.normals | (0 << 0);

                                    renderingData_.vertices.push_back(aux);

                                }

                                // Add texture to the face.
                                models::addTexture(*bNeighbor, renderingData_.vertices);

                            }

                        }

                    }

            }

            if (nBlocksMinusX_) {

                for (y = 0; y < SCY; y++)
                    for (z = 0; z < SCZ; z++) {

                        // Add block's model to the mesh if necessary.
                        if (!blocksLocalIDs[0][y][z]) {

                            if (neighborLocalID = neighborBlocksMinusX_[y][z]) {

                                bNeighbor = &block::getBlockC(palette_.getT2(neighborLocalID));

                                // Create the face's vertices for face x+.
                                for (int vertex = 0; vertex < blockTriangles_->operator[](0).size(); vertex++) {

                                    aux.positions[0] = (renderingData_.chunkPos.x - 1) * SCX + 15 + blockVertices_->operator[](blockTriangles_->operator[](5)[vertex]).positions[0];
                                    aux.positions[1] = renderingData_.chunkPos.y * SCY + y + blockVertices_->operator[](blockTriangles_->operator[](5)[vertex]).positions[1];
                                    aux.positions[2] = renderingData_.chunkPos.z * SCZ + z + blockVertices_->operator[](blockTriangles_->operator[](5)[vertex]).positions[2];

                                    aux.normals = 0 | (0 << 30);
                                    aux.normals = aux.normals | (512 << 20);
                                    aux.normals = aux.normals | (512 << 10);
                                    aux.normals = aux.normals | (1023 << 0);

                                    renderingData_.vertices.push_back(aux);

                                }

                                // Add texture to the face.
                                models::addTexture(*bNeighbor, renderingData_.vertices);

                            }

                        }

                    }

            }

            // Read chunk data section ends.
            blocksMutex_.unlock_shared();

        }

        return renderingData_.vertices.size();

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

        std::memset(blocksLocalIDs, 0, nBlocksChunk * sizeof(unsigned short));

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
        const vec3& chunkPos = renderingData_.chunkPos;
        chunkManager::onUnloadAsFrontier(vec3{ chunkPos.x+1, chunkPos.y, chunkPos.z });
        chunkManager::onUnloadAsFrontier(vec3{ chunkPos.x-1, chunkPos.y, chunkPos.z });
        chunkManager::onUnloadAsFrontier(vec3{ chunkPos.x, chunkPos.y+1, chunkPos.z });
        chunkManager::onUnloadAsFrontier(vec3{ chunkPos.x, chunkPos.y-1, chunkPos.z });
        chunkManager::onUnloadAsFrontier(vec3{ chunkPos.x, chunkPos.y, chunkPos.z+1 });
        chunkManager::onUnloadAsFrontier(vec3{ chunkPos.x, chunkPos.y, chunkPos.z-1 });
    
    }

    chunk::~chunk() {}

    void chunk::reset() {
    
        blockVertices_ = nullptr;
        blockTriangles_ = nullptr;

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
            if (world::isSaved(chunk_->chunkPos())) { // Load previously saved chunk.

                std::string data = world::loadChunk(chunk_->chunkPos());
                std::string word;
                char c = 0;
                unsigned int index = 0,
                             nBytes = data.size();
                const char* dataBegin = &data[index];
                unsigned short localID = 0;
                unsigned int globalID = 0;
                unsigned short count = 0;

                chunk_->blockDataMutex().lock();
                std::memcpy(chunk_->blocks(), dataBegin, sizeof(unsigned short) * nBlocksChunk);
                index += sizeof(unsigned short) * nBlocksChunk + 1;

                std::memcpy(chunk_->neighborBlocksPlusX(), &data[index], sizeof(unsigned short) * nBlocksChunkEdge);
                index += sizeof(unsigned short) * nBlocksChunkEdge + 1;

                std::memcpy(chunk_->neighborBlocksMinusX(), &data[index], sizeof(unsigned short) * nBlocksChunkEdge);
                index += sizeof(unsigned short) * nBlocksChunkEdge + 1;

                std::memcpy(chunk_->neighborBlocksPlusY(), &data[index], sizeof(unsigned short) * nBlocksChunkEdge);
                index += sizeof(unsigned short) * nBlocksChunkEdge + 1;

                std::memcpy(chunk_->neighborBlocksMinusY(), &data[index], sizeof(unsigned short) * nBlocksChunkEdge);
                index += sizeof(unsigned short) * nBlocksChunkEdge + 1;

                std::memcpy(chunk_->neighborBlocksPlusZ(), &data[index], sizeof(unsigned short) * nBlocksChunkEdge);
                index += sizeof(unsigned short) * nBlocksChunkEdge + 1;

                std::memcpy(chunk_->neighborBlocksMinusZ(), &data[index], sizeof(unsigned short) * nBlocksChunkEdge);
                index += sizeof(unsigned short) * nBlocksChunkEdge + 1;

                palette<unsigned short, unsigned int>& chunkPalette = chunk_->getPalette();
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

                std::unordered_map<unsigned short, unsigned short>& chunkPaletteCount = chunk_->getPaletteCount();
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
                chunk_->blockDataMutex().unlock();

                c = data[++index]; // Skip the '@' delimiter character
                unsigned int state = 0;
                while (index < nBytes) {
                
                    while (c != '|') {

                        word += c;

                        c = data[++index];

                    }

                    switch (state) {
                    
                    case 0:
                        chunk_->nBlocks(sto<unsigned short>(word));
                        break;
                    case 1:
                        chunk_->nBlocksPlusX(sto<unsigned short>(word));
                        break;
                    case 2:
                        chunk_->nBlocksMinusX(sto<unsigned short>(word));
                        break;
                    case 3:
                        chunk_->nBlocksPlusY(sto<unsigned short>(word));
                        break;
                    case 4:
                        chunk_->nBlocksMinusY(sto<unsigned short>(word));
                        break;
                    case 5:
                        chunk_->nBlocksPlusZ(sto<unsigned short>(word));
                        break;
                    case 6:
                        chunk_->nBlocksMinusZ(sto<unsigned short>(word));
                        break;
                    
                    }

                    word.clear();
                    c = data[++index];
                    state++;
                
                }

                chunk_->needsRemesh(true); // EL BUG ES QUE SI MODIFICO UN BLOCK EN UN BORDE, TAMBIEN HAY QUE GUARDAR EL CHUNK VECINO QUE LE HACE FRONTERA.

            }
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
    std::unordered_map<vec3, model>* chunkManager::chunkMeshesUpdated_,
                                   * chunkManager::chunkMeshesWrite_,
                                   * chunkManager::chunkMeshesRead_;
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

    vertexBuffer* chunkManager::vbo_ = nullptr;

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

            chunkMeshesUpdated_ = new std::unordered_map<vec3, model>;
            chunkMeshesWrite_ = new std::unordered_map<vec3, model>;
            chunkMeshesRead_ = new std::unordered_map<vec3, model>;

            // NEW
            chunkTasks_ = new threadPool(MAX_N_CHUNK_SIMULT_TASKS);
            priorityChunkTasks_ = new threadPool(MAX_N_CHUNK_SIMULT_TASKS);
            loadChunkJobs_ = new atomicRecyclingPool<chunkJob>(MAX_N_CHUNK_SIMULT_TASKS);
            loadChunkJobs_->setAllFreeOnClear(true);
            chunksPool_.setAllFreeOnClear(false);
            vbo_ = &graphics::vbo("chunks");
            vbo_->bind();
            vbo_->prepareDynamic(nMaxChunkVertsToCompute() * sizeof(vertex));

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
            chunkMeshesUpdated_->operator[](data.chunkPos) = data.vertices;
            it = priorityNewChunkMeshes_.erase(it);
                
            c->unlockSharedRenderingDataMutex();

        }

        *chunkMeshesWrite_ = *chunkMeshesUpdated_;

    }

    void chunkManager::updateReadChunkMeshes(std::unique_lock<std::mutex>& priorityUpdatesLock) {

        chunk* c = nullptr;

        for (auto it = chunkMeshesUpdated_->begin(); it != chunkMeshesUpdated_->end();) {

            playerChunkPosCopy_ = camera::cPlayerCamera()->chunkPos();

            while (priorityUpdatesRemaining_)
                priorityUpdatesRemainingCV_.wait(priorityUpdatesLock);

            if (!chunkInRenderDistance(it->first))
                it = chunkMeshesUpdated_->erase(it);
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
                model& vertices = c->renderingData().vertices;
                if (vertices.size())
                    chunkMeshesUpdated_->operator[](c->chunkPos()) = vertices;
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
    
        std::unordered_map<vec3, model>* aux = chunkMeshesWrite_;
        chunkMeshesWrite_ = chunkMeshesRead_;
        chunkMeshesRead_ = aux;
    
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
                    selectedBlock = &AgentChunk[chunkPos]->getBlock(floorMod(posX, SCX), floorMod(posY, SCY), floorMod(posZ, SCZ));

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
                    selectedBlock = &AgentChunk[chunkPos]->getBlock(floorMod(posX, SCX), floorMod(posY, SCY), floorMod(posZ, SCZ));

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

    vec3 chunkManager::chunkDistanceToPlayer(const vec3& chunkPos) {

        const vec3& playerPos = player::getCamera().chunkPos();
        return vec3{ (float)std::abs(chunkPos.x - playerPos.x),
                     (float)std::abs(chunkPos.y - playerPos.y),
                     (float)std::abs(chunkPos.z - playerPos.z) };

    }

    vec3 chunkManager::chunkDistance(const vec3& chunkPos1, const vec3& chunkPos2) {
    
        return vec3{ (float)std::abs(chunkPos1.x - chunkPos2.x),
                     (float)std::abs(chunkPos1.y - chunkPos2.y),
                     (float)std::abs(chunkPos1.z - chunkPos2.z) };
    
    }

    double chunkManager::distanceToPlayer(const vec3& chunkPos) {
    
        const vec3& playerPos = player::getCamera().chunkPos();
        double distanceX = playerPos.x - (chunkPos.x + 0.5) * SCX;
        double distanceZ = playerPos.y - (chunkPos.y + 0.5) * SCY;
        double distanceY = playerPos.z - (chunkPos.z + 0.5) * SCZ;
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

                
                ensureChunkIfVisible(playerChunkPosCopy_.x, playerChunkPosCopy_.y, playerChunkPosCopy_.z); // ALSO NEXT. ASEGURARSE DE QUE ESTE CHUNK SEA EL DEL JUGADOR YA POSICIONADO BIEN TRAS CARGA DE MUNDO.

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
;
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

        data += std::to_string(c->nBlocks()) + '|' + std::to_string(c->nBlocksPlusX()) + '|' + std::to_string(c->nBlocksMinusX()) + '|' + std::to_string(c->nBlocksPlusY()) + '|' + std::to_string(c->nBlocksMinusY()) + '|' + std::to_string(c->nBlocksPlusZ()) + '|' + std::to_string(c->nBlocksMinusZ()) + '|';

        c->blockDataMutex().unlock_shared();

        t.finish();
        logger::debugLog("Time for saving chunks is " + std::to_string(t.getDurationMs()));

        return data;
    
    }

    chunk* chunkManager::loadChunk(const vec3& chunkPos) {

        chunk* c = &chunksPool_.get();

        c->chunkPos() = chunkPos;

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

        initialised_ = false;
        
    }

    const block& chunkManager::getBlockOGWorld_(int posX, int posY, int posZ) {
    
        vec3 chunkPos = getChunkCoords(posX, posY, posZ);

        if (clientChunks_.find(chunkPos) == clientChunks_.end())
            logger::errorLog("Chunk " + std::to_string(chunkPos.x) + "|" + std::to_string(chunkPos.y) + "|" + std::to_string(chunkPos.z) + " does not exist");
        else
            return clientChunks_[chunkPos]->getBlock(floorMod(posX, SCX), floorMod(posY, SCY), floorMod(posZ, SCZ));
    
    }

}
