#include "entity.h"
#include <stdexcept>
#include "graphics.h"
#include "model.h"
#include "utilities.h"

#include <iostream>
#include <ostream>

// 'player' class

player::player(float FOV, float zNear, float zFar, VoxelEng::window& window,
               unsigned int blockReachRange, const glm::vec3& position, unsigned int spawnWorldID,
               atomic<bool>* appFinished, const glm::vec3& direction)
	: window_(window.windowAPIpointer()), camera_(FOV, zNear, zFar, window, position, direction),
    blockReachRange_(blockReachRange), blockSearchIncrement_(0.10f), chunkMng_(nullptr), selectedBlock_(0),
    selectedBlockPos_(glm::vec3(0, 0, 0)), oldSelectedBlockPos_(glm::vec3(0,0,0)), appFinished_(appFinished),
    destroyBlock_(false), placeBlock_(false)
{

    if (!appFinished_)
        throw runtime_error("appFinished's address was null when creating an object of class 'player'");

}

void player::selectBlock()
{

    float step = blockSearchIncrement_;
    selectedBlock_ = 0;


    while (step < blockReachRange_ && !selectedBlock_)
    {

        selectedBlockPos_ = camera_.pos() + (camera_.direction() * step);

        // MAKE THIS SEARCH ONLY IN THE 4 POSSIBLE NEIGHBORS, NOT IN THE 8 POSSIBLE ONES.
        selectedBlock_ = chunkMng_->getBlock(floor(selectedBlockPos_.x), floor(selectedBlockPos_.y), floor(selectedBlockPos_.z));

        if (!selectedBlock_)
        {

            oldSelectedBlockPos_ = selectedBlockPos_;

            step += blockSearchIncrement_;

        }

    }

}

void player::mouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
{

    VoxelEng::graphics::getPlayerCallbackPtr()->mouseButtonHandler(window, button, action, mods);

}

void player::mouseButtonHandler(GLFWwindow* window, int button, int action, int mods)
{

    // If left mouse button is pressed, destroy selected block at selected position.
    destroyBlock_ = button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS;

    // Right mouse button is pressed, place selected block at selected position.
    placeBlock_ = button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS;

}

void player::destroySelectedBlock()
{

    unique_lock<recursive_mutex> lock(chunkMng_->chunksMutex());

    chunk* selectedChunk = chunkMng_->selectChunkByRealPos(selectedBlockPos_);

    if (selectedChunk && selectedBlock_) 
    {

        chunkRelativePos chunkRelPos(VoxelEng::floorMod(floor(selectedBlockPos_.x), SCX),
                                     VoxelEng::floorMod(floor(selectedBlockPos_.y), SCY),
                                     VoxelEng::floorMod(floor(selectedBlockPos_.z), SCZ));


        selectedChunk->blockDataMutex().lock();
    
        selectedChunk->setBlock(chunkRelPos, 0);

        selectedChunk->blockDataMutex().unlock();

        chunkMng_->highPriorityUpdate(selectedChunk->chunkPos());

        if (chunkRelPos.x == 0)
            chunkMng_->highPriorityUpdate(chunkMng_->neighborMinusX(selectedChunk->chunkPos())->chunkPos());

        if (chunkRelPos.x == 15)
            chunkMng_->highPriorityUpdate(chunkMng_->neighborPlusX(selectedChunk->chunkPos())->chunkPos());

        if (chunkRelPos.y == 0)
            chunkMng_->highPriorityUpdate(chunkMng_->neighborMinusY(selectedChunk->chunkPos())->chunkPos());

        if (chunkRelPos.y == 15)
            chunkMng_->highPriorityUpdate(chunkMng_->neighborPlusY(selectedChunk->chunkPos())->chunkPos());

        if (chunkRelPos.z == 0)
            chunkMng_->highPriorityUpdate(chunkMng_->neighborMinusZ(selectedChunk->chunkPos())->chunkPos());

        if (chunkRelPos.z == 15)
            chunkMng_->highPriorityUpdate(chunkMng_->neighborPlusZ(selectedChunk->chunkPos())->chunkPos());

    }

}

void player::placeSelectedBlock()
{

    VoxelEng::block blockToPlace = 2;


    unique_lock<recursive_mutex> lock(chunkMng_->chunksMutex());

    chunk* selectedChunk = chunkMng_->selectChunkByRealPos(oldSelectedBlockPos_);

    if (selectedChunk && selectedBlock_)
    {

        chunkRelativePos chunkRelPos(VoxelEng::floorMod(floor(oldSelectedBlockPos_.x), SCX),
                                     static_cast<int>(oldSelectedBlockPos_.y) % SCY,
                                     VoxelEng::floorMod(floor(oldSelectedBlockPos_.z), SCZ));


        selectedChunk->blockDataMutex().lock();

        selectedChunk->setBlock(chunkRelPos, blockToPlace);

        selectedChunk->blockDataMutex().unlock();

        chunkMng_->highPriorityUpdate(selectedChunk->chunkPos());

        if (chunkRelPos.x == 0)
            chunkMng_->highPriorityUpdate(chunkMng_->neighborMinusX(selectedChunk->chunkPos())->chunkPos());

        if (chunkRelPos.x == 15)
            chunkMng_->highPriorityUpdate(chunkMng_->neighborPlusX(selectedChunk->chunkPos())->chunkPos());

        if (chunkRelPos.y == 0)
            chunkMng_->highPriorityUpdate(chunkMng_->neighborMinusY(selectedChunk->chunkPos())->chunkPos());

        if (chunkRelPos.y == 15)
            chunkMng_->highPriorityUpdate(chunkMng_->neighborPlusY(selectedChunk->chunkPos())->chunkPos());

        if (chunkRelPos.z == 0)
            chunkMng_->highPriorityUpdate(chunkMng_->neighborMinusZ(selectedChunk->chunkPos())->chunkPos());

        if (chunkRelPos.z == 15)
            chunkMng_->highPriorityUpdate(chunkMng_->neighborPlusZ(selectedChunk->chunkPos())->chunkPos());

    }

}

void player::processPlayerInput()
{

    while(!*appFinished_)
    {
    
        selectBlock();

        if (destroyBlock_)
        { 

            destroySelectedBlock();
            destroyBlock_ = false;

        }
        else
            if (placeBlock_)
            {

                placeSelectedBlock();
                placeBlock_ = false;

            }
    
    }

}

namespace VoxelEng {

    entity::entity(unsigned int modelID, float x, float y, float z) : model_(models::getModel(modelID)), x_(x), y_(y), z_(z) {

        // Translate the model's copy to the entity's position.
        for (unsigned int i = 0; i < model_.size(); i++) {
        
            model_[i].positions[0] += x_;
            model_[i].positions[1] += y_;
            model_[i].positions[2] += z_;
        
        }
    
    }

}