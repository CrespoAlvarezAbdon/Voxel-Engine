#include "chunk.h"
#include <algorithm>
#include <ctime>
#include <cstdlib>
#include <cmath>
#include <chrono>
#include <cstddef>
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
      neighborPlusY_(nullptr),
      neighborMinusY_(nullptr),
      neighborPlusX_(nullptr),
      neighborMinusX_(nullptr),
      neighborPlusZ_(nullptr),
      neighborMinusZ_(nullptr),
      loadLevel_(chunkLoadLevel::NOTLOADED) {
    
        renderingData_.chunkPos = vec3Zero;

        for (int x = 0; x < SCX; x++)
            for (int y = 0; y < SCY; y++)
                for (int z = 0; z < SCZ; z++)
                    blocksNew_[x][y][z] = 0;
    
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
      loadLevel_(chunkLoadLevel::NOTLOADED)
    {

        renderingData_.chunkPos = chunkPos;

        for (int x = 0; x < SCX; x++)
            for (int y = 0; y < SCY; y++)
                for (int z = 0; z < SCZ; z++)
                    blocksNew_[x][y][z] = 0;
        
        if (!empty)
            worldGen::generate(*this); // This can only call to setBlock to modify the chunk and that method already takes care of 'blocksMutex_'.
            
        // Link with existing neighbors.
        if (neighborPlusY_ = chunkManager::selectChunk(chunkPos.x, chunkPos.y + 1, chunkPos.z))
            neighborPlusY_->neighborMinusY_ = this;

        if (neighborMinusY_ = chunkManager::selectChunk(chunkPos.x, chunkPos.y - 1, chunkPos.z))
            neighborMinusY_->neighborPlusY_ = this;

        if (neighborPlusX_ = chunkManager::selectChunk(chunkPos.x + 1, chunkPos.y, chunkPos.z))
            neighborPlusX_->neighborMinusX_ = this;

        if (neighborMinusX_ = chunkManager::selectChunk(chunkPos.x - 1, chunkPos.y, chunkPos.z))
            neighborMinusX_->neighborPlusX_ = this;

        if (neighborPlusZ_ = chunkManager::selectChunk(chunkPos.x, chunkPos.y, chunkPos.z + 1))
            neighborPlusZ_->neighborMinusZ_ = this;

        if (neighborMinusZ_ = chunkManager::selectChunk(chunkPos.x, chunkPos.y, chunkPos.z - 1))
            neighborMinusZ_->neighborPlusZ_ = this;
        
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
      neighborPlusY_(c.neighborPlusY_),
      neighborMinusY_(c.neighborMinusY_),
      neighborPlusX_(c.neighborPlusX_),
      neighborMinusX_(c.neighborMinusX_),
      neighborPlusZ_(c.neighborPlusZ_),
      neighborMinusZ_(c.neighborMinusZ_),
      nNeighbors_(c.nNeighbors_) {

        std::unique_lock<std::shared_mutex> lock(blocksMutex_);

        palette_ = c.palette_;
        paletteCount_ = c.paletteCount_;
        freeLocalIDs_ = c.freeLocalIDs_;
        for (GLbyte x = 0; x < SCX; x++)
            for (GLbyte y = 0; y < SCY; y++)
                for (GLbyte z = 0; z < SCZ; z++)
                    blocksNew_[x][y][z] = c.blocksNew_[x][y][z];

        renderingData_.chunkPos = c.renderingData_.chunkPos;
        renderingData_.vertices = c.renderingData_.vertices;

        if (neighborPlusY_)
            neighborPlusY_->neighborMinusY_ = this;

        if (neighborMinusY_)
            neighborMinusY_->neighborPlusY_ = this;

        if (neighborPlusX_)
            neighborPlusX_->neighborMinusX_ = this;

        if (neighborMinusX_)
            neighborMinusX_->neighborPlusX_ = this;

        if (neighborPlusZ_)
            neighborPlusZ_->neighborMinusZ_ = this;

        if (neighborMinusZ_)
            neighborMinusZ_->neighborPlusZ_ = this;

    }

    const block& chunk::getBlock(GLbyte x, GLbyte y, GLbyte z) {

        std::unique_lock<std::shared_mutex> lock(blocksMutex_);

        unsigned int localID = blocksNew_[x][y][z];

        return localID ? block::getBlockC(palette_.getT2(localID)) : block::emptyBlock();

    }

    const block& chunk::setBlock(GLbyte x, GLbyte y, GLbyte z, const block& b, bool modification) {

        std::unique_lock<std::shared_mutex> lock(blocksMutex_);

        unsigned short newLocalID = 0,
                       oldLocalID = blocksNew_[x][y][z];
        unsigned int newGlobalID = b.intID(),
                     oldGlobalID = oldLocalID ? palette_.getT2(oldLocalID) : 0;
        needsRemesh_ = oldGlobalID != newGlobalID;

        // Increase/decrease the palette's size.
        if (needsRemesh_) {

            modified_ = modified_ || modification;

            // The old local ID is no longer used at (x,y,z).
            if (oldLocalID) {
            
                if (paletteCount_[oldLocalID] == 1) {

                    palette_.eraseT1(oldLocalID);
                    paletteCount_.erase(oldLocalID);
                    freeLocalIDs_.insert(oldLocalID);

                }
                else
                    paletteCount_[oldLocalID]--;
            
            }

            if (newGlobalID == 0)// The new block is an empty block.
                blocksNew_[x][y][z] = 0;
            else {
            
                if (palette_.containsT2(newGlobalID)) { // The new block already has a relation in the palette.

                    newLocalID = palette_.getT1(newGlobalID);
                    paletteCount_[newLocalID]++;

                }
                else { // The new block does not have a related local ID in the palette.

                    if (freeLocalIDs_.empty()) {

                        newLocalID = palette_.size() + 1;
                        palette_.insert(newLocalID, newGlobalID); // METER UNA RECYCLING POOL PARA REUSAR LOCAL IDs

                    }
                    else {

                        newLocalID = *freeLocalIDs_.begin();
                        freeLocalIDs_.erase(newLocalID);

                    }

                }

                blocksNew_[x][y][z] = newLocalID;

            }

            // Increase/decrease the chunk's block counters.
            if (newLocalID && !oldLocalID) {

                nBlocks_++;

                if (x == 15)
                    nBlocksPlusX_++;
                else if (x == 0)
                    nBlocksMinusX_++;
                else if (y == 15)
                    nBlocksPlusY_++;
                else if (y == 0)
                    nBlocksMinusY_++;
                else if (z == 15)
                    nBlocksPlusZ_++;
                else if (z == 0)
                    nBlocksMinusZ_++;

            }
            else if (!newLocalID && oldLocalID) {

                nBlocks_--;

                if (x == 15)
                    nBlocksPlusX_--;
                else if (x == 0)
                    nBlocksMinusX_--;
                else if (y == 15)
                    nBlocksPlusY_--;
                else if (y == 0)
                    nBlocksMinusY_--;
                else if (z == 15)
                    nBlocksPlusZ_--;
                else if (z == 0)
                    nBlocksMinusZ_--;

            }
        
        }

        return block::getBlockC(oldGlobalID);

    }

    bool chunk::renewMesh() {

        std::unique_lock<std::recursive_mutex> lock(renderingDataMutex_);

        if (needsRemesh_) {

            needsRemesh_ = false;

            renderingDataMutex_.lock();

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
            if (nBlocks_ > 0 && nBlocks_ < nBlocksChunk) // Do not iterate through this if there are no blocks to render faces that belong to this chunk.
                for (x = 0; x < SCX; x++)
                    for (y = 0; y < SCY; y++)
                        for (z = 0; z < SCZ; z++) {

                            // Add block's model to the mesh if necessary.
                            if (!blocksNew_[x][y][z]) {

                                // Front face vertices with culling of non-visible faces. z+
                                if (z < SCZ - 1 && (neighborLocalID = blocksNew_[x][y][z + 1])) {

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
                                if (z > 0 && (neighborLocalID = blocksNew_[x][y][z - 1])) {

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
                                if (y < SCY - 1 && (neighborLocalID = blocksNew_[x][y + 1][z])) {

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
                                if (y > 0 && (neighborLocalID = blocksNew_[x][y - 1][z])) {

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
                                if (x < SCX - 1 && (neighborLocalID = blocksNew_[x + 1][y][z])) {

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
                                if (x > 0 && (neighborLocalID = blocksNew_[x - 1][y][z])) {

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
 
            // Render faces that require data from neighbor chunks.
            if (neighborPlusX_ && neighborPlusX_->loadLevel_.load() >= chunkLoadLevel::DECORATED) {

                x = SCX - 1;
                for (y = 0; y < SCY; y++)
                    for (z = 0; z < SCZ; z++) {

                        if (neighborPlusX_->blocksNew_[0][y][z] && !blocksNew_[x][y][z]) {
                        
                            // Create the face's vertices for face x-.
                            for (int vertex = 0; vertex < blockTriangles_->operator[](0).size(); vertex++) {

                                aux.positions[0] = neighborPlusX_->renderingData_.chunkPos.x * SCX + blockVertices_->operator[](blockTriangles_->operator[](4)[vertex]).positions[0];
                                aux.positions[1] = neighborPlusX_->renderingData_.chunkPos.y * SCY + y + blockVertices_->operator[](blockTriangles_->operator[](4)[vertex]).positions[1];
                                aux.positions[2] = neighborPlusX_->renderingData_.chunkPos.z * SCZ + z + blockVertices_->operator[](blockTriangles_->operator[](4)[vertex]).positions[2];

                                aux.normals = 0 | (0 << 30);
                                aux.normals = aux.normals | (512 << 20);
                                aux.normals = aux.normals | (512 << 10);
                                aux.normals = aux.normals | (0 << 0);

                                renderingData_.vertices.push_back(aux); // Upload the mesh data to the chunk that is being meshed now, not its neighbor.

                            }

                            // Add texture to the face.
                            models::addTexture(block::getBlockC(neighborPlusX_->palette_.getT2(neighborPlusX_->blocksNew_[0][y][z])), renderingData_.vertices);
                        
                        }
                        else if (!neighborPlusX_->blocksNew_[0][y][z] && blocksNew_[x][y][z]) {
                        
                            // Create the face's vertices for face x+.
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
                            models::addTexture(block::getBlockC(palette_.getT2(blocksNew_[x][y][z])), renderingData_.vertices);
                        
                        }

                    }

            }

            if (neighborMinusX_ && neighborMinusX_->loadLevel_.load() >= chunkLoadLevel::DECORATED) {

                x = 0;
                for (y = 0; y < SCY; y++)
                    for (z = 0; z < SCZ; z++) {

                        if (neighborMinusX_->blocksNew_[SCX-1][y][z] && !blocksNew_[x][y][z]) {

                            // Create the face's vertices for face x+.
                            for (int vertex = 0; vertex < blockTriangles_->operator[](0).size(); vertex++) {

                                aux.positions[0] = neighborMinusX_->renderingData_.chunkPos.x * SCX + SCX-1 + blockVertices_->operator[](blockTriangles_->operator[](5)[vertex]).positions[0];
                                aux.positions[1] = neighborMinusX_->renderingData_.chunkPos.y * SCY + y + blockVertices_->operator[](blockTriangles_->operator[](5)[vertex]).positions[1];
                                aux.positions[2] = neighborMinusX_->renderingData_.chunkPos.z * SCZ + z + blockVertices_->operator[](blockTriangles_->operator[](5)[vertex]).positions[2];

                                aux.normals = 0 | (0 << 30);
                                aux.normals = aux.normals | (512 << 20);
                                aux.normals = aux.normals | (512 << 10);
                                aux.normals = aux.normals | (1023 << 0);

                                renderingData_.vertices.push_back(aux);

                            }

                            // Add texture to the face.
                            models::addTexture(block::getBlockC(neighborMinusX_->palette_.getT2(neighborMinusX_->blocksNew_[SCX - 1][y][z])), renderingData_.vertices);

                        }
                        else if (!neighborMinusX_->blocksNew_[SCX-1][y][z] && blocksNew_[x][y][z]) {

                            // Create the face's vertices for face x-.
                            for (int vertex = 0; vertex < blockTriangles_->operator[](0).size(); vertex++) {

                                aux.positions[0] = renderingData_.chunkPos.x * SCX + blockVertices_->operator[](blockTriangles_->operator[](4)[vertex]).positions[0];
                                aux.positions[1] = renderingData_.chunkPos.y * SCY + y + blockVertices_->operator[](blockTriangles_->operator[](4)[vertex]).positions[1];
                                aux.positions[2] = renderingData_.chunkPos.z * SCZ + z + blockVertices_->operator[](blockTriangles_->operator[](4)[vertex]).positions[2];

                                aux.normals = 0 | (0 << 30);
                                aux.normals = aux.normals | (512 << 20);
                                aux.normals = aux.normals | (512 << 10);
                                aux.normals = aux.normals | (0 << 0);

                                renderingData_.vertices.push_back(aux); // Upload the mesh data to the chunk that is being meshed now, not its neighbor.

                            }

                            // Add texture to the face.
                            models::addTexture(block::getBlockC(palette_.getT2(blocksNew_[x][y][z])), renderingData_.vertices);

                        }

                    }

            }

            if (neighborPlusY_ && neighborPlusY_->loadLevel_.load() >= chunkLoadLevel::DECORATED) {

                y = SCY - 1;
                for (x = 0; x < SCX; x++)
                    for (z = 0; z < SCZ; z++) {

                        if (neighborPlusY_->blocksNew_[x][0][z] && !blocksNew_[x][y][z]) {

                            // Create the face's vertices for face y-.
                            for (int vertex = 0; vertex < blockTriangles_->operator[](0).size(); vertex++) {

                                aux.positions[0] = neighborPlusY_->renderingData_.chunkPos.x * SCX + x + blockVertices_->operator[](blockTriangles_->operator[](3)[vertex]).positions[0];
                                aux.positions[1] = neighborPlusY_->renderingData_.chunkPos.y * SCY + blockVertices_->operator[](blockTriangles_->operator[](3)[vertex]).positions[1];
                                aux.positions[2] = neighborPlusY_->renderingData_.chunkPos.z * SCZ + z + blockVertices_->operator[](blockTriangles_->operator[](3)[vertex]).positions[2];

                                aux.normals = 0 | (0 << 30);
                                aux.normals = aux.normals | (512 << 20);
                                aux.normals = aux.normals | (0 << 10);
                                aux.normals = aux.normals | (512 << 0);

                                renderingData_.vertices.push_back(aux);

                            }

                            // Add texture to the face.
                            models::addTexture(block::getBlockC(neighborPlusY_->palette_.getT2(neighborPlusY_->blocksNew_[x][0][z])), renderingData_.vertices);

                        }
                        else if (!neighborPlusY_->blocksNew_[x][0][z] && blocksNew_[x][y][z]) {

                            // Create the face's vertices for face y+.
                            for (int vertex = 0; vertex < blockTriangles_->operator[](0).size(); vertex++) {

                                aux.positions[0] = renderingData_.chunkPos.x * SCX + x + blockVertices_->operator[](blockTriangles_->operator[](2)[vertex]).positions[0];
                                aux.positions[1] = renderingData_.chunkPos.y * SCY + y + blockVertices_->operator[](blockTriangles_->operator[](2)[vertex]).positions[1];
                                aux.positions[2] = renderingData_.chunkPos.z * SCY + z + blockVertices_->operator[](blockTriangles_->operator[](2)[vertex]).positions[2];

                                aux.normals = 0 | (0 << 30);
                                aux.normals = aux.normals | (512 << 20);
                                aux.normals = aux.normals | (1023 << 10);
                                aux.normals = aux.normals | (512 << 0);

                                renderingData_.vertices.push_back(aux);

                            }

                            // Add texture to the face.
                            models::addTexture(block::getBlockC(palette_.getT2(blocksNew_[x][y][z])), renderingData_.vertices);

                        }

                    }

            }

            if (neighborMinusY_ && neighborMinusY_->loadLevel_.load() >= chunkLoadLevel::DECORATED) {

                y = 0;
                for (x = 0; x < SCX; x++)
                    for (z = 0; z < SCZ; z++) {

                        if (neighborMinusY_->blocksNew_[x][SCY - 1][z] && !blocksNew_[x][y][z]) {

                            // Create the face's vertices for face y+.
                            for (int vertex = 0; vertex < blockTriangles_->operator[](0).size(); vertex++) {

                                aux.positions[0] = neighborMinusY_->renderingData_.chunkPos.x * SCX + x + blockVertices_->operator[](blockTriangles_->operator[](2)[vertex]).positions[0];
                                aux.positions[1] = neighborMinusY_->renderingData_.chunkPos.y * SCY + SCY-1 + blockVertices_->operator[](blockTriangles_->operator[](2)[vertex]).positions[1];
                                aux.positions[2] = neighborMinusY_->renderingData_.chunkPos.z * SCZ + z + blockVertices_->operator[](blockTriangles_->operator[](2)[vertex]).positions[2];

                                aux.normals = 0 | (0 << 30);
                                aux.normals = aux.normals | (512 << 20);
                                aux.normals = aux.normals | (1023 << 10);
                                aux.normals = aux.normals | (512 << 0);

                                renderingData_.vertices.push_back(aux);

                            }

                            // Add texture to the face.
                            models::addTexture(block::getBlockC(neighborMinusY_->palette_.getT2(neighborMinusY_->blocksNew_[x][SCY - 1][z])), renderingData_.vertices);

                        }
                        else if (!neighborMinusY_->blocksNew_[x][SCY - 1][z] && blocksNew_[x][y][z]) {

                            // Create the face's vertices for face y-.
                            for (int vertex = 0; vertex < blockTriangles_->operator[](0).size(); vertex++) {

                                aux.positions[0] = renderingData_.chunkPos.x * SCX + x + blockVertices_->operator[](blockTriangles_->operator[](3)[vertex]).positions[0];
                                aux.positions[1] = renderingData_.chunkPos.y * SCY + blockVertices_->operator[](blockTriangles_->operator[](3)[vertex]).positions[1];
                                aux.positions[2] = renderingData_.chunkPos.z * SCZ + z + blockVertices_->operator[](blockTriangles_->operator[](3)[vertex]).positions[2];

                                aux.normals = 0 | (0 << 30);
                                aux.normals = aux.normals | (512 << 20);
                                aux.normals = aux.normals | (0 << 10);
                                aux.normals = aux.normals | (512 << 0);

                                renderingData_.vertices.push_back(aux);

                            }

                            // Add texture to the face.
                            models::addTexture(block::getBlockC(palette_.getT2(blocksNew_[x][y][z])), renderingData_.vertices);

                        }

                    }

            }

            if (neighborPlusZ_ && neighborPlusZ_->loadLevel_.load() >= chunkLoadLevel::DECORATED) {

                z = SCZ - 1;
                for (x = 0; x < SCX; x++)
                    for (y = 0; y < SCY; y++) {

                        if (neighborPlusZ_->blocksNew_[x][y][0] && !blocksNew_[x][y][z]) {

                            // Create the face's vertices for face z-.
                            for (int vertex = 0; vertex < blockTriangles_->operator[](0).size(); vertex++) {

                                aux.positions[0] = neighborPlusZ_->renderingData_.chunkPos.x * SCX + x + blockVertices_->operator[](blockTriangles_->operator[](0)[vertex]).positions[0];
                                aux.positions[1] = neighborPlusZ_->renderingData_.chunkPos.y * SCY + y + blockVertices_->operator[](blockTriangles_->operator[](0)[vertex]).positions[1];
                                aux.positions[2] = neighborPlusZ_->renderingData_.chunkPos.z * SCZ + blockVertices_->operator[](blockTriangles_->operator[](0)[vertex]).positions[2];

                                aux.normals = 0 | (0 << 30);
                                aux.normals = aux.normals | (0 << 20);
                                aux.normals = aux.normals | (512 << 10);
                                aux.normals = aux.normals | (512 << 0);

                                renderingData_.vertices.push_back(aux);

                            }

                            // Add texture to the face.
                            models::addTexture(block::getBlockC(neighborPlusZ_->palette_.getT2(neighborPlusZ_->blocksNew_[x][y][0])), renderingData_.vertices);

                        }
                        else if (!neighborPlusZ_->blocksNew_[x][y][0] && blocksNew_[x][y][z]) {

                            // Create the face's vertices for z+.
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
                            models::addTexture(block::getBlockC(palette_.getT2(blocksNew_[x][y][z])), renderingData_.vertices);

                        }

                    }

            }

            if (neighborMinusZ_ && neighborMinusZ_->loadLevel_.load() >= chunkLoadLevel::DECORATED) {

                z = 0;
                for (x = 0; x < SCX; x++)
                    for (y = 0; y < SCY; y++) {

                        if (neighborMinusZ_->blocksNew_[x][y][SCZ - 1] && !blocksNew_[x][y][z]) {

                            // Create the face's vertices for z+.
                            for (int vertex = 0; vertex < blockTriangles_->operator[](0).size(); vertex++) {

                                aux.positions[0] = neighborMinusZ_->renderingData_.chunkPos.x * SCX + x + blockVertices_->operator[](blockTriangles_->operator[](1)[vertex]).positions[0];
                                aux.positions[1] = neighborMinusZ_->renderingData_.chunkPos.y * SCY + y + blockVertices_->operator[](blockTriangles_->operator[](1)[vertex]).positions[1];
                                aux.positions[2] = neighborMinusZ_->renderingData_.chunkPos.z * SCZ + SCZ - 1 + blockVertices_->operator[](blockTriangles_->operator[](1)[vertex]).positions[2];

                                aux.normals = 0 | (0 << 30);
                                aux.normals = aux.normals | (1023 << 20);
                                aux.normals = aux.normals | (512 << 10);
                                aux.normals = aux.normals | (512 << 0);

                                renderingData_.vertices.push_back(aux);

                            }

                            // Add texture to the face.
                            models::addTexture(block::getBlockC(neighborMinusZ_->palette_.getT2(neighborMinusZ_->blocksNew_[x][y][SCZ - 1])), renderingData_.vertices);

                        }
                        else if (!neighborMinusZ_->blocksNew_[x][y][SCZ - 1] && blocksNew_[x][y][z]) {

                            // Create the face's vertices for face z-.
                            for (int vertex = 0; vertex < blockTriangles_->operator[](0).size(); vertex++) {

                                aux.positions[0] = renderingData_.chunkPos.x * SCX + x + blockVertices_->operator[](blockTriangles_->operator[](0)[vertex]).positions[0];
                                aux.positions[1] = renderingData_.chunkPos.y * SCY + y + blockVertices_->operator[](blockTriangles_->operator[](0)[vertex]).positions[1];
                                aux.positions[2] = renderingData_.chunkPos.z * SCZ + blockVertices_->operator[](blockTriangles_->operator[](0)[vertex]).positions[2];

                                aux.normals = 0 | (0 << 30);
                                aux.normals = aux.normals | (0 << 20);
                                aux.normals = aux.normals | (512 << 10);
                                aux.normals = aux.normals | (512 << 0);

                                renderingData_.vertices.push_back(aux);

                            }

                            // Add texture to the face.
                            models::addTexture(block::getBlockC(palette_.getT2(blocksNew_[x][y][z])), renderingData_.vertices);

                        }

                    }

            }

            // Read chunk data section ends.
            blocksMutex_.unlock_shared();

            renderingDataMutex_.unlock();

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

        for (GLbyte x = 0; x < SCX; x++)
            for (GLbyte y = 0; y < SCY; y++)
                for (GLbyte z = 0; z < SCZ; z++)
                    blocksNew_[x][y][z] = 0;

    }

    void chunk::regenChunk(bool empty, const vec3& chunkPos) {
    
        makeEmpty();
        renderingData_.chunkPos = chunkPos;
        
        if (!empty)
            worldGen::generate(*this);

        // Link with existing neighbors.
        if (neighborPlusY_ = chunkManager::selectChunk(chunkPos.x, chunkPos.y + 1, chunkPos.z))
            neighborPlusY_->neighborMinusY_ = this;

        if (neighborMinusY_ = chunkManager::selectChunk(chunkPos.x, chunkPos.y - 1, chunkPos.z))
            neighborMinusY_->neighborPlusY_ = this;

        if (neighborPlusX_ = chunkManager::selectChunk(chunkPos.x + 1, chunkPos.y, chunkPos.z))
            neighborPlusX_->neighborMinusX_ = this;

        if (neighborMinusX_ = chunkManager::selectChunk(chunkPos.x - 1, chunkPos.y, chunkPos.z))
            neighborMinusX_->neighborPlusX_ = this;

        if (neighborPlusZ_ = chunkManager::selectChunk(chunkPos.x, chunkPos.y, chunkPos.z + 1))
            neighborPlusZ_->neighborMinusZ_ = this;

        if (neighborMinusZ_ = chunkManager::selectChunk(chunkPos.x, chunkPos.y, chunkPos.z - 1))
            neighborMinusZ_->neighborPlusZ_ = this;
    
    }

    void chunk::unlinkNeighbors() {
    
        if (neighborPlusY_) {
        
            neighborPlusY_->neighborMinusY_ = nullptr;

            neighborPlusY_ = nullptr;
        
        }
            
        if (neighborMinusY_) {

            neighborMinusY_->neighborPlusY_ = nullptr;

            neighborMinusY_ = nullptr;

        }

        if (neighborPlusX_) {

            neighborPlusX_->neighborMinusX_ = nullptr;

            neighborPlusX_ = nullptr;

        }

        if (neighborMinusX_) {

            neighborMinusX_->neighborPlusX_ = nullptr;
 
            neighborMinusX_ = nullptr;

        }

        if (neighborPlusZ_) {

            neighborPlusZ_->neighborMinusZ_ = nullptr;

            neighborPlusZ_ = nullptr;

        }

        if (neighborMinusZ_ ) {

            neighborMinusZ_->neighborPlusZ_ = nullptr;

            neighborMinusZ_ = nullptr;

        }

        nNeighbors_ = 0;
    
    }

    int chunk::onUnloadAsFrontier() {
    
        // Check for the neighbors of this recently unloaded frontier chunk

        double distanceUnloadedFrontier = chunkManager::distanceToPlayer(renderingData_.chunkPos);
        int nLoadedFrontiers =
            chunkManager::onUnloadAsFrontier(neighborPlusX_, distanceUnloadedFrontier) +
            chunkManager::onUnloadAsFrontier(neighborMinusX_, distanceUnloadedFrontier) +
            chunkManager::onUnloadAsFrontier(neighborPlusY_, distanceUnloadedFrontier) +
            chunkManager::onUnloadAsFrontier(neighborMinusY_, distanceUnloadedFrontier) +
            chunkManager::onUnloadAsFrontier(neighborPlusZ_, distanceUnloadedFrontier) +
            chunkManager::onUnloadAsFrontier(neighborMinusZ_, distanceUnloadedFrontier);

        return nLoadedFrontiers;
    
    }

    void chunk::setNeighbors() {
    
        // Link with existing neighbors.
        if (neighborPlusY_ = chunkManager::selectChunk(renderingData_.chunkPos.x, renderingData_.chunkPos.y + 1, renderingData_.chunkPos.z))
            neighborPlusY_->neighborMinusY_ = this;

        if (neighborMinusY_ = chunkManager::selectChunk(renderingData_.chunkPos.x, renderingData_.chunkPos.y - 1, renderingData_.chunkPos.z))
            neighborMinusY_->neighborPlusY_ = this;

        if (neighborPlusX_ = chunkManager::selectChunk(renderingData_.chunkPos.x + 1, renderingData_.chunkPos.y, renderingData_.chunkPos.z))
            neighborPlusX_->neighborMinusX_ = this;
        nNeighbors_ += neighborPlusX_ != nullptr;

        if (neighborMinusX_ = chunkManager::selectChunk(renderingData_.chunkPos.x - 1, renderingData_.chunkPos.y, renderingData_.chunkPos.z))
            neighborMinusX_->neighborPlusX_ = this;

        if (neighborPlusZ_ = chunkManager::selectChunk(renderingData_.chunkPos.x, renderingData_.chunkPos.y, renderingData_.chunkPos.z + 1))
            neighborPlusZ_->neighborMinusZ_ = this;

        if (neighborMinusZ_ = chunkManager::selectChunk(renderingData_.chunkPos.x, renderingData_.chunkPos.y, renderingData_.chunkPos.z - 1))
            neighborMinusZ_->neighborPlusZ_ = this;
    
    }

    chunk::~chunk() {
   


    }

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

    void meshChunkJob::setAttributes(chunk* chunk, atomicRecyclingPool<meshChunkJob>* pool, bool generateTerrain, bool isPriorityUpdate,
        std::condition_variable* priorityNewChunkMeshesCV) {

        chunk_ = chunk;
        pool_ = pool;
        generateTerrain_ = generateTerrain;
        isPriorityUpdate_ = isPriorityUpdate;
        priorityNewChunkMeshesCV_ = priorityNewChunkMeshesCV;

    }

    void meshChunkJob::process() {

        if (generateTerrain_)
            worldGen::generate(*chunk_);
        chunkManager::renewMesh(chunk_, isPriorityUpdate_);

        if (isPriorityUpdate_)
            priorityNewChunkMeshesCV_->notify_all();

        pool_->free(*this);

    }


    // 'chunkManager' class.

    // NEW STARTS HERE

    bool chunkManager::removeFrontierIt_;
    vec3 chunkManager::playerChunkPosCopy_;
    std::list<vec3> chunkManager::frontierChunks_;
    std::list<vec3>::iterator chunkManager::frontierIt_;
    chunkManager::notInRenderDistance chunkManager::notInRenderDistance_;
    chunkManager::closestChunk chunkManager::closestChunk_;
    threadPool* chunkManager::chunkTasks_ = nullptr,
              * chunkManager::priorityChunkTasks_ = nullptr;
    atomicRecyclingPool<meshChunkJob>* chunkManager::loadChunkJobs_;
    atomicRecyclingPool<chunk> chunkManager::chunksPool_;
    chunkEvent chunkManager::onChunkLoad_("On chunk load");
    chunkEvent chunkManager::onChunkUnload_("On chunk unload");
    vertexBuffer* chunkManager::vbo_ = nullptr;
    std::atomic<bool> chunkManager::clearChunksFlag_ = false,
                      chunkManager::priorityUpdatesRemaining_ = false;

    // NEW ENDS HERE

    bool chunkManager::initialised_ = false;
    int chunkManager::nChunksToCompute_ = 0;
    std::unordered_map<vec3, chunk*> chunkManager::chunks_;
    std::unordered_map<vec3, model>* chunkManager::drawableChunksRead_;
    std::list<chunk*> chunkManager::newChunkMeshes_,
                      chunkManager::priorityNewChunkMeshes_;

    std::list<vec3> chunkManager::chunkMeshesToDelete_;

    std::mutex chunkManager::managerThreadMutex_,
               chunkManager::priorityManagerThreadMutex_,
               chunkManager::priorityNewChunkMeshesMutex_,
               chunkManager::priorityUpdatesRemainingMutex_,
               chunkManager::loadingTerrainMutex_;
    std::recursive_mutex chunkManager::newChunkMeshesMutex_,
                         chunkManager::chunkMeshesToDeleteMutex_,
                         chunkManager::chunksMutex_;
    std::condition_variable chunkManager::managerThreadCV_,
                            chunkManager::priorityManagerThreadCV_,
                            chunkManager::priorityNewChunkMeshesCV_,
                            chunkManager::loadingTerrainCV_;
    std::condition_variable_any chunkManager::priorityUpdatesRemainingCV_;

    std::atomic<bool> chunkManager::waitInitialTerrainLoaded_ = true;

    std::string chunkManager::openedTerrainFileName_ = "";

    std::unordered_map<unsigned int, std::unordered_map<vec3, bool>> chunkManager::AIChunkAvailable_;
    std::unordered_map<unsigned int, std::unordered_map<vec3, chunk*>> chunkManager::AIagentChunks_;
    unsigned int chunkManager::selectedAIWorld_ = 0;

    bool chunkManager::originalWorldAccess_ = true;

    void chunkManager::init() {

        if (initialised_)
            logger::errorLog("Chunk management system was already initialised");
        else {

            waitInitialTerrainLoaded_ = true;

            openedTerrainFileName_ = "";

            originalWorldAccess_ = true;

            selectedAIWorld_ = 0;

            drawableChunksRead_ = new std::unordered_map<vec3, model>;

            // NEW
            chunkTasks_ = new threadPool(MAX_N_CHUNK_SIMULT_TASKS);
            priorityChunkTasks_ = new threadPool(MAX_N_CHUNK_SIMULT_TASKS);
            loadChunkJobs_ = new atomicRecyclingPool<meshChunkJob>(MAX_N_CHUNK_SIMULT_TASKS);
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
            it = priorityNewChunkMeshes_.erase(it);

            c->lockRenderingDataMutex();
            model& vertices = c->renderingData().vertices;
            if (vertices.size())
                drawableChunksRead_->operator[](c->chunkPos()) = vertices;
            c->unlockRenderingDataMutex();

        }

    }

    void chunkManager::updateReadChunkMeshes(std::unique_lock<std::mutex>& priorityUpdatesLock) {

        chunk* c = nullptr;

        newChunkMeshesMutex_.lock();
        for (auto it = newChunkMeshes_.begin(); it != newChunkMeshes_.end();) {

            while (priorityUpdatesRemaining_)
                priorityUpdatesRemainingCV_.wait(priorityUpdatesLock);

            c = *it;
            c->lockRenderingDataMutex();
            model& vertices = c->renderingData().vertices;
            if (vertices.size())
                drawableChunksRead_->operator[](c->chunkPos()) = vertices;
            c->unlockRenderingDataMutex();

            it = newChunkMeshes_.erase(it);

        }
        newChunkMeshesMutex_.unlock();

        chunkMeshesToDeleteMutex_.lock();
        // NEXT
        // PASAR CHUNKMESHESTODELETE Y NEWCHUNKSMESHES A LISTAS PARA QUITAR O(N) Y HACER PROFILING DEL CHUNK MANAGER
        for (auto it = chunkMeshesToDelete_.begin(); it != chunkMeshesToDelete_.end();) {

            while (priorityUpdatesRemaining_)
                priorityUpdatesRemainingCV_.wait(priorityUpdatesLock);

            drawableChunksRead_->erase(*it);

            it = chunkMeshesToDelete_.erase(it);

        }
        chunkMeshesToDeleteMutex_.unlock();

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

        return chunks_.find(chunkPos) != chunks_.cend();

    }

    chunkLoadLevel chunkManager::getChunkLoadLevel(const vec3& chunkPos) {

        std::unique_lock<std::recursive_mutex> lock(chunksMutex_);

        return (chunks_.find(chunkPos) != chunks_.cend()) ? chunks_[chunkPos]->loadLevel() : chunkLoadLevel::NOTLOADED;

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
            
                if (chunks_.find(chunkPos) == chunks_.cend())
                    logger::errorLog("There is no chunk " + std::to_string(chunkPos.x) + '|' + std::to_string(chunkPos.y) + '|' + std::to_string(chunkPos.z) +
                                     "for AI agent " + std::to_string(selectedAIWorld_));
                else {
                    
                    agentWorld[chunkPos] = new chunk(*chunks_[chunkPos]);
                    AIChunkAvailable_[selectedAIWorld_][chunkPos] = true;
                
                }
                
            }
            
            return agentWorld[chunkPos]->setBlock(getChunkRelCoords(x, y, z), blockID); // Chunks are not rendered in AI mode.
        
        }
        else {
        
            if (chunks_.find(chunkPos) == chunks_.cend())
                logger::errorLog("Chunk " + std::to_string(chunkPos.x) + "|" + std::to_string(chunkPos.y) + "|" + std::to_string(chunkPos.z) + " does not exist");
            else {
            
                block removedBlock = chunks_[chunkPos]->setBlock(getChunkRelCoords(x, y, z), blockID);

                return removedBlock;
            
            }
                 
        }

    }

    chunk* chunkManager::selectChunk(const vec3& chunkPos) {

        std::unique_lock<std::recursive_mutex> lock(chunksMutex_);

        auto it = chunks_.find(chunkPos);
        if (it != chunks_.end())
            return it->second;
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

    chunk* chunkManager::selectChunkOrCreate(int x, int y, int z) {

        vec3 chunkPos{ x, y, z };
        chunk* c = nullptr;
        std::unique_lock<std::recursive_mutex> lock(chunksMutex_);

        if (chunks_.find(chunkPos) == chunks_.end()) {
        
            c = new chunk(false, chunkPos);
            chunks_.insert({ chunkPos, c });
        
        }
        else
            c = chunks_.at(chunkPos);

        c->needsRemesh() = true;

        return c;

    }

    chunk* chunkManager::selectChunkOrCreate(const vec3& chunkPos) {

        chunk* c = nullptr;
        std::unique_lock<std::recursive_mutex> lock(chunksMutex_);

        if (chunks_.find(chunkPos) == chunks_.end()) {

            c = new chunk(false, chunkPos);
            chunks_.insert({ chunkPos, c });

        }
        else
            c = chunks_.at(chunkPos);

        c->needsRemesh() = true;

        return c;

    }

    chunk* chunkManager::neighborMinusX(const vec3& chunkPos) {

        vec3 neighborChunkPos{ chunkPos.x - 1, chunkPos.y, chunkPos.z };


        std::unique_lock<std::recursive_mutex> lock(chunksMutex_);

        if (chunks_.find(neighborChunkPos) != chunks_.end())
            return chunks_.at(neighborChunkPos);
        else
            return nullptr;

    }

    chunk* chunkManager::neighborPlusX(const vec3& chunkPos) {

        vec3 neighborChunkPos{ chunkPos.x + 1, chunkPos.y, chunkPos.z };


        std::unique_lock<std::recursive_mutex> lock(chunksMutex_);

        if (chunks_.find(neighborChunkPos) != chunks_.end())
            return chunks_.at(neighborChunkPos);
        else
            return nullptr;

    }

    chunk* chunkManager::neighborMinusY(const vec3& chunkPos) {

        vec3 neighborChunkPos{ chunkPos.x, chunkPos.y - 1, chunkPos.z };


        std::unique_lock<std::recursive_mutex> lock(chunksMutex_);

        if (chunks_.find(neighborChunkPos) != chunks_.end())
            return chunks_.at(neighborChunkPos);
        else
            return nullptr;

    }

    chunk* chunkManager::neighborPlusY(const vec3& chunkPos) {

        vec3 neighborChunkPos{ chunkPos.x, chunkPos.y + 1, chunkPos.z };


        std::unique_lock<std::recursive_mutex> lock(chunksMutex_);

        if (chunks_.find(neighborChunkPos) != chunks_.end())
            return chunks_.at(neighborChunkPos);
        else
            return nullptr;

    }

    chunk* chunkManager::neighborMinusZ(const vec3& chunkPos) {

        vec3 neighborChunkPos{ chunkPos.x, chunkPos.y, chunkPos.z - 1 };


        std::unique_lock<std::recursive_mutex> lock(chunksMutex_);

        if (chunks_.find(neighborChunkPos) != chunks_.end())
            return chunks_.at(neighborChunkPos);
        else
            return nullptr;

    }

    chunk* chunkManager::neighborPlusZ(const vec3& chunkPos) {

        vec3 neighborChunkPos{ chunkPos.x, chunkPos.y, chunkPos.z + 1 };


        std::unique_lock<std::recursive_mutex> lock(chunksMutex_);

        if (chunks_.find(neighborChunkPos) != chunks_.end())
            return chunks_.at(neighborChunkPos);
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

    int chunkManager::unloadFrontierChunk(const vec3& chunkPos) {

        std::unique_lock<std::recursive_mutex> lock(chunksMutex_);
        
        std::unordered_map<vec3, chunk*>::iterator it = chunks_.find(chunkPos);

        if (it == chunks_.end())
            logger::errorLog("Chunk at " + std::to_string(chunkPos.x) + "|" + std::to_string(chunkPos.y) + "|" + std::to_string(chunkPos.z)
                + " is not registered");
        else {

            chunk* unloadedChunk = it->second;
            
            // Check if the chunk's neighbors become frontier chunks after it is unloaded.
            int nNewFrontierChunks = unloadedChunk->onUnloadAsFrontier();

            frontierChunks_.erase(std::find(frontierChunks_.begin(), frontierChunks_.end(), chunkPos));

            chunks_.erase(chunkPos);

            chunkMeshesToDeleteMutex_.lock();
            chunkMeshesToDelete_.push_back(chunkPos);
            chunkMeshesToDeleteMutex_.unlock();

            if (unloadedChunk->modified())
                world::saveChunk(unloadedChunk);

            chunksPool_.free(*unloadedChunk);
            // NEXT
            // unloadedChunk->getPalette().erase(); // o algo así
            // averiguar por qué narices se quedan chunks descolgados
            unloadedChunk->makeEmpty();
            unloadedChunk->setLoadLevel(chunkLoadLevel::NOTLOADED);
            unloadedChunk->unlinkNeighbors();
            unloadedChunk->modified() = false;

            onChunkUnload_.notify(chunkPos.x, chunkPos.z);

            return nNewFrontierChunks;

            return 0;

        }
        
    }

    bool chunkManager::onUnloadAsFrontier(chunk* chunk, double distUnloadedToPlayer) {

        if (chunk && std::find(frontierChunks_.begin(), frontierChunks_.end(), chunk->chunkPos()) == frontierChunks_.end()) {

            frontierChunks_.push_back(chunk->chunkPos());
            return true;

        }
        else
            return false;
    
    }

    void chunkManager::addFrontier(chunk* chunk) {
    
        // NEXT. MIRAR SI BASTANTES CHECKS DE STD::FIND DE FRONTIER CHUNKS SE PUEDEN SUSTITUIR POR COMPROBACIONES EN CHUNKS_ O EN OTRAS COSAS MÁS LIGERAS.
        if (std::find(frontierChunks_.begin(), frontierChunks_.end(), chunk->chunkPos()) == frontierChunks_.end())
            frontierChunks_.push_back(chunk->chunkPos());

        updateFrontierNeighbor(chunk, chunk->neighborPlusX());
        updateFrontierNeighbor(chunk, chunk->neighborMinusX());
        updateFrontierNeighbor(chunk, chunk->neighborPlusY());
        updateFrontierNeighbor(chunk, chunk->neighborMinusY());
        updateFrontierNeighbor(chunk, chunk->neighborPlusZ());
        updateFrontierNeighbor(chunk, chunk->neighborMinusZ());
    
    }

    void chunkManager::renewMesh(chunk* chunk, bool isPriorityUpdate) {
    
        if (chunk->renewMesh()) {
        
            if (isPriorityUpdate) {
            
                priorityNewChunkMeshesMutex_.lock();

                priorityNewChunkMeshes_.push_back(chunk);

                priorityNewChunkMeshesMutex_.unlock();
            
            }
            else {
            
                newChunkMeshesMutex_.lock();

                newChunkMeshes_.push_back(chunk);

                newChunkMeshesMutex_.unlock();
            
            }

        }
        
    }

    void chunkManager::renewMesh(const vec3& chunkPos, bool isPriorityUpdate) {
    
        chunksMutex_.lock();

        auto it = chunks_.find(chunkPos);
        if (it == chunks_.cend()) {

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
            // TODO. ADD PROPER WORLDGENERATOR SELECTION.
            if (!VoxelEng::worldGen::isGenRegistered("miningWorldGen"))
                VoxelEng::worldGen::registerGen<AIExample::miningWorldGen>("miningWorldGen");
            VoxelEng::worldGen::selectGen("miningWorldGen");
            worldGen::prepareGen();

            player::changeTransform(worldGen::playerSpawnPos());

            // First of all, load the chunk where the player is in.
            playerChunkPosCopy_ = camera::cPlayerCamera()->chunkPos();
            ensureChunkIfVisible(playerChunkPosCopy_.x, playerChunkPosCopy_.y, playerChunkPosCopy_.z);
            timer t;
            while (game::threadsExecute[2]) {

                bool continueCreatingChunks = false;

                playerChunkPosCopy_ = camera::cPlayerCamera()->chunkPos();

                do {

                    // Unload chunks that are outside the maximun render distance.
                    t.start();
                    frontierIt_ = frontierChunks_.begin();
                    while (frontierIt_ != frontierChunks_.end()) {

                        while (priorityUpdatesRemaining_)
                            priorityUpdatesRemainingCV_.wait(priorityUpdatesLock);                           

                        const vec3 chunkPos = *frontierIt_++;

                        if (!chunkInRenderDistance(chunkPos))
                            unloadFrontierChunk(chunkPos);

                    }
                    t.finish();
                    logger::debugLog("Time T1 is " + std::to_string(t.getDurationMs()));
                    //logger::print("\r frontier size is" + std::to_string(frontierChunks_.size()));
                    
                    // Load new chunks that are inside render distance if necessary.
                    // Mark frontier chunks that are no longer frontier and delete them.
                    t.start();
                    frontierChunks_.sort(closestChunk_);
                    continueCreatingChunks = false;
                    unsigned int nIterations = 0;
                    const unsigned int maxIterations = 32;
                    for (frontierIt_ = frontierChunks_.begin(); frontierIt_ != frontierChunks_.end() && nIterations < maxIterations;) {

                        const vec3 chunkPos = *frontierIt_;

                        while (priorityUpdatesRemaining_)
                            priorityUpdatesRemainingCV_.wait(priorityUpdatesLock);

                        // Ensure chunks are loaded and meshed if visible.
                        if (ensureChunkIfVisible(chunkPos.x + 1, chunkPos.y, chunkPos.z) ||
                            ensureChunkIfVisible(chunkPos.x - 1, chunkPos.y, chunkPos.z) ||
                            ensureChunkIfVisible(chunkPos.x, chunkPos.y + 1, chunkPos.z) ||
                            ensureChunkIfVisible(chunkPos.x, chunkPos.y - 1, chunkPos.z) ||
                            ensureChunkIfVisible(chunkPos.x, chunkPos.y, chunkPos.z + 1) ||
                            ensureChunkIfVisible(chunkPos.x, chunkPos.y, chunkPos.z - 1)) {

                            continueCreatingChunks = ++nIterations < maxIterations;

                        }

                        if (removeFrontierIt_) {

                            frontierIt_ = frontierChunks_.erase(frontierIt_);

                            removeFrontierIt_ = false;

                        }
                        else
                            frontierIt_++;

                    }

                    t.finish();
                    logger::debugLog("Time T2 is " + std::to_string(t.getDurationMs()));

                } while (continueCreatingChunks);

                if (waitInitialTerrainLoaded_) {
                
                    waitInitialTerrainLoaded_ = false;
                    loadingTerrainCV_.notify_one();
                
                }

                //logger::print("\r Chunks " + std::to_string(chunks_.size()));
                t.start();
                updateReadChunkMeshes(priorityUpdatesLock);
                managerThreadCV_.wait(lock);
                t.finish();
                logger::debugLog("Time T3 is " + std::to_string(t.getDurationMs()));

                // Clear all the chunks in case it is necessary
                if (false) {
                
                    chunkTasks_->awaitNoJobs();
                    priorityChunkTasks_->awaitNoJobs();
                    frontierChunks_.clear();
                    loadChunkJobs_->waitUntilAllFree();
                    chunksPool_.clear();

                    chunksMutex_.lock();
                    for (auto it = chunks_.begin(); it != chunks_.end(); it++)
                        if (it->second)
                            delete it->second;
                    chunks_.clear();
                    chunksMutex_.unlock();

                    priorityNewChunkMeshes_.clear();
                    newChunkMeshes_.clear();
                    
                    // Begin the chunk loading process again by loading the chunk where the player is in first.
                    ensureChunkIfVisible(playerChunkPosCopy_.x, playerChunkPosCopy_.y, playerChunkPosCopy_.z);

                    clearChunksFlag_ = false;
                
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
                updatePriorityReadChunkMeshes();
                priorityManagerThreadCV_.wait(lock);
                priorityUpdatesRemaining_ = false;
                priorityUpdatesRemainingCV_.notify_all();

                priorityNewChunkMeshesCV_.wait(lockNewChunksMeshes);

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
                            chunks_.insert_or_assign(chunkPos, selectedChunk);

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
        
        chunksMutex_.lock();
        if (chunkInRenderDistance(chunkPos) && !chunks_.contains(chunkPos)) {

            chunksMutex_.unlock();
            return loadChunkV2(chunkPos) != nullptr;

        }
        else {
        
            chunksMutex_.unlock();
            return false;
        
        }
        
    }

    chunk* chunkManager::loadChunkV2(const vec3& chunkPos) {
    
        chunk* c = &chunksPool_.get();
        c->makeEmpty();
        c->chunkPos() = chunkPos;
        c->setNeighbors();
        c->setLoadLevel(chunkLoadLevel::NOTLOADED);

        chunksMutex_.lock();
        chunks_[chunkPos] = c;
        chunksMutex_.unlock();

        addFrontier(c);

        onChunkLoad_.notify(chunkPos.x, chunkPos.z);

        // Submit async task to load the chunk either from disk or by generating it.
        issueChunkMeshJob(c, true, false);

        return c;
    
    }

    void chunkManager::issueChunkMeshJob(chunk* c, bool generateTerrain, bool isPriorityUpdate) {
    
        meshChunkJob* aJob = &loadChunkJobs_->get();
        aJob->setAttributes(c, loadChunkJobs_, generateTerrain, isPriorityUpdate, &priorityNewChunkMeshesCV_);

        if (isPriorityUpdate) {
        
            c->needsRemesh() = true;
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
        for (auto it = chunks_.begin(); it != chunks_.end(); it++)
            if (it->second)
                delete it->second;
        chunks_.clear();
        chunksMutex_.unlock();

        if (drawableChunksRead_)
            drawableChunksRead_->clear();

        chunksPool_.clear();

        priorityNewChunkMeshesMutex_.lock();
        priorityNewChunkMeshes_.clear();
        priorityNewChunkMeshesMutex_.unlock();

        newChunkMeshesMutex_.lock();
        newChunkMeshes_.clear();
        newChunkMeshesMutex_.unlock();

        chunkMeshesToDeleteMutex_.lock();
        chunkMeshesToDelete_.clear();
        chunkMeshesToDeleteMutex_.unlock();

        frontierChunks_.clear();

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

        if (drawableChunksRead_) {

            delete drawableChunksRead_;
            drawableChunksRead_ = nullptr;

        }

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

        initialised_ = false;
        
    }

    void chunkManager::updateFrontierNeighbor(chunk* frontierChunk, chunk* neighborChunk) {
    
        if (neighborChunk) {
        
            frontierChunk->nNeighbors()++;

            // If a frontier is surrounded in all possible directions, then it is no longer a frontier
            auto it = std::find(frontierChunks_.begin(), frontierChunks_.end(), neighborChunk->chunkPos()); // NEXTSE PODRÍA PONER UN IT DENTRO DE CADA CHUNK PARA ACCEDER A SU POS EN LA LISTA
            if (++neighborChunk->nNeighbors() >= 6 && it != frontierChunks_.end()) {
            
                if (it == frontierIt_)
                    removeFrontierIt_ = true;
                else
                    frontierChunks_.erase(it);
            
            
            }

        }
    
    }

    const block& chunkManager::getBlockOGWorld_(int posX, int posY, int posZ) {
    
        vec3 chunkPos = getChunkCoords(posX, posY, posZ);

        if (chunks_.find(chunkPos) == chunks_.end())
            logger::errorLog("Chunk " + std::to_string(chunkPos.x) + "|" + std::to_string(chunkPos.y) + "|" + std::to_string(chunkPos.z) + " does not exist");
        else
            return chunks_[chunkPos]->getBlock(floorMod(posX, SCX), floorMod(posY, SCY), floorMod(posZ, SCZ));
    
    }

}
