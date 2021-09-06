#include "entity.h"

#include <iostream>
#include <ostream>
using namespace std;

// TODO. ADD SAFE nullptr CHECKING.


// 'player' class

player::player(float FOV, float width, float height, float zNear, float zFar, GLFWwindow* window, 
               unsigned int blockReachRange, const glm::vec3& position, const glm::vec3& direction)
	: window_(window), camera_(FOV, width, height, zNear, zFar, window, position, direction),
    blockReachRange_(blockReachRange), blockSearchIncrement_(0.10f), chunkMng_(nullptr), selectedBlock_(0),
    selectedBlockPos_(glm::vec3(0, 0, 0)), oldSelectedBlockPos_(glm::vec3(0,0,0))
{}

void player::selectBlock()
{

    float step = blockSearchIncrement_;
    selectedBlock_ = 0;


    while (step < blockReachRange_ && !selectedBlock_)
    {

        selectedBlockPos_ = camera_.pos() + (camera_.direction() * step);

        selectedBlock_ = chunkMng_->getBlock(selectedBlockPos_);

        if (!selectedBlock_)
        {

            oldSelectedBlockPos_ = selectedBlockPos_;

            step += blockSearchIncrement_;

        }

    }

    cout << "\r[DEBUG]: looking at: " << selectedBlock_;

}

void player::mouseButtonHandler(GLFWwindow* window, int button, int action, int mods)
{

    // Left mouse button is pressed.
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) 
    {
    
        // Destroy selected block.
        if (selectedBlock_)
        {

            destroySelectedBlock();

        }
    
    }

}

void player::destroySelectedBlock()
{

    // TODO. ADD THIS MODIFIER.
    unique_lock<recursive_mutex> lock(chunkMng_->chunksMutex());

    chunk* selectedChunk = chunkMng_->selectChunkByRealPos(selectedBlockPos_);


    if (selectedChunk) 
    {
    
        selectedChunk->blockDataMutex().lock();
    
        selectedChunk->setBlock(static_cast<int>(selectedBlockPos_.x) % SCX,
                                static_cast<int>(selectedBlockPos_.y) % SCY,
                                static_cast<int>(selectedBlockPos_.z) % SCZ,
                                0);

        selectedChunk->blockDataMutex().unlock();

        chunkMng_->highPriorityUpdate(selectedChunk->chunkPos());

    }

}