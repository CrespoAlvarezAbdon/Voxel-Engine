#include "player.h"

#include "game.h"
#include "gui.h"

namespace VoxelEng {

    // 'player' class.

    bool player::initialised_ = false,
         player::moveUp_ = false,
         player::moveDown_ = false,
         player::moveNorth_ = false,
         player::moveSouth_ = false,
         player::moveEast_ = false,
         player::moveWest_ = false,
         player::rollRight_ = false,
         player::rollLeft_ = false;
    window* player::window_ = nullptr;
    camera* player::camera_ = nullptr;
    float player::blockReachRange_ = 0.0f,
          player::blockSearchIncrement_ = 0.0f,
          player::movementSpeed_ = 0.0f,
          player::rollSpeed_ = 0.0f,
          player::pitchViewDir_ = 0.0f,
          player::yawViewDir_ = 0.0f,
          player::mouseSensibility_ = 0.0f;
    double player::mouseX_ = 0.0,
           player::mouseY_ = 0.0,
           player::oldMouseX_ = 0.0,
           player::oldMouseY_ = 0.0;
    const block* player::selectedBlock_ = nullptr;
    std::atomic<const block*> player::blockToPlace_ = nullptr;
    vec3 player::selectedBlockPos_ = vec3Zero,
         player::oldSelectedBlockPos_ = vec3Zero,
         player::pitchAxis_ = vec3Zero,
         player::yawAxis_ = vec3Zero,
         player::rollAxis_ = vec3Zero;
    std::atomic<bool> player::destroyBlock_ = false,
                      player::placeBlock_ = false;
    entity* player::playerEntity_ = nullptr;
    transform* player::playerTransform_ = nullptr;


    void player::init(float FOV, float zNear, float zFar, window& window, unsigned int blockReachRange) {

        if (initialised_)
            logger::errorLog("Player system is already initialised");
        else {

            if (game::selectedEngineMode() == engineMode::AIMENULOOP) {

                moveUp_ = false;
                moveDown_ = false;
                moveNorth_ = false;
                moveSouth_ = false;
                moveEast_ = false;
                moveWest_ = false;
                rollRight_ = false;
                rollLeft_ = false;
                window_ = &window;
                camera_ = new camera(FOV, zNear, zFar, window, true);
                blockReachRange_ = blockReachRange;
                blockSearchIncrement_ = 0.1f;
                movementSpeed_ = 10.0f;
                rollSpeed_ = 70.0f;
                pitchViewDir_ = 0.0f;
                yawViewDir_ = 0.0f;
                mouseSensibility_ = 0.25f;
                mouseX_ = 0.0;
                mouseY_ = 0.0;
                oldMouseX_ = 0.0;
                oldMouseY_ = 0.0;
                selectedBlock_ = block::emptyBlockP();
                blockToPlace_ = block::emptyBlockP();
                selectedBlockPos_ = vec3Zero;
                oldSelectedBlockPos_ = vec3Zero;
                pitchAxis_ = vec3Zero;
                yawAxis_ = vec3Zero;
                rollAxis_ = vec3Zero;
                destroyBlock_ = false;
                placeBlock_ = false;
                playerEntity_ = &entityManager::getEntity(entityManager::registerEntity(2, vec3Zero, vec3Zero, nullptr));  // TODO. ADD PROPER PLAYER MODEL.
                playerTransform_ = &playerEntity_->getTransform();

                initialised_ = true;

            }
            else
                logger::errorLog("The player class must be initialised in the AI menu loop");

        }

    }

    void player::selectBlock() {

        float step = blockSearchIncrement_;
        const vec3& dir = playerTransform_->viewDirection,
                    globalPos = camera_->globalPos();
        vec3 blockPos;
        selectedBlock_ = block::emptyBlockP();

        while (step < blockReachRange_ && selectedBlock_->isEmptyBlock()) {

            selectedBlockPos_ = globalPos + (dir * step);

            blockPos.x = floor(selectedBlockPos_.x);
            blockPos.y = floor(selectedBlockPos_.y);
            blockPos.z = floor(selectedBlockPos_.z);

            selectedBlock_ = (chunkManager::getChunkLoadLevel(getChunkCoords(blockPos)) == chunkStatus::DECORATED) ? &chunkManager::getBlock(blockPos) : block::emptyBlockP();

            if (selectedBlock_->isEmptyBlock()) { // No non-empty block found. Continue searching.

                oldSelectedBlockPos_ = selectedBlockPos_;

                step += blockSearchIncrement_;

            }

        }

    }

    void player::mouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {

        // If left mouse button is pressed, destroy selected block at selected position.
        destroyBlock_ = button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS;

        // Right mouse button is pressed, place selected block at selected position.
        placeBlock_ = button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS;

    }

    void player::destroySelectedBlock() {

        if (!GUImanager::levelGUIOpened()) {

            std::unique_lock<std::recursive_mutex> lock(chunkManager::chunksMutex());

            chunk* selectedChunk = chunkManager::selectChunkByRealPos(selectedBlockPos_),
                 * neighbor = nullptr;

            if (selectedChunk && !selectedBlock_->isEmptyBlock()) {

                const vec3& chunkPos = selectedChunk->chunkPos();
                vec3 chunkRelPos = getChunkRelCoords(selectedBlockPos_);

                selectedChunk->setBlock(chunkRelPos, block::emptyBlock());

                chunkManager::issueChunkMeshJob(selectedChunk, chunkJobType::PRIORITYREMESH);

                if (chunkRelPos.x == 0 && (neighbor = chunkManager::neighborMinusX(chunkPos))) {

                    neighbor->setBlockNeighbor(chunkRelPos.y, chunkRelPos.z, blockViewDir::PLUSX, block::emptyBlock());
                    chunkManager::issueChunkMeshJob(neighbor, chunkJobType::PRIORITYREMESH);

                }

                if (chunkRelPos.x == 15 && (neighbor = chunkManager::neighborPlusX(chunkPos))) {

                    neighbor->setBlockNeighbor(chunkRelPos.y, chunkRelPos.z, blockViewDir::NEGX, block::emptyBlock());
                    chunkManager::issueChunkMeshJob(neighbor, chunkJobType::PRIORITYREMESH);

                }

                if (chunkRelPos.y == 0 && (neighbor = chunkManager::neighborMinusY(chunkPos))) {

                    neighbor->setBlockNeighbor(chunkRelPos.x, chunkRelPos.z, blockViewDir::PLUSY, block::emptyBlock());
                    chunkManager::issueChunkMeshJob(neighbor, chunkJobType::PRIORITYREMESH);

                }

                if (chunkRelPos.y == 15 && (neighbor = chunkManager::neighborPlusY(chunkPos))) {

                    neighbor->setBlockNeighbor(chunkRelPos.x, chunkRelPos.z, blockViewDir::NEGY, block::emptyBlock());
                    chunkManager::issueChunkMeshJob(neighbor, chunkJobType::PRIORITYREMESH);

                }

                if (chunkRelPos.z == 0 && (neighbor = chunkManager::neighborMinusZ(chunkPos))) {

                    neighbor->setBlockNeighbor(chunkRelPos.x, chunkRelPos.y, blockViewDir::PLUSZ, block::emptyBlock());
                    chunkManager::issueChunkMeshJob(neighbor, chunkJobType::PRIORITYREMESH);

                }

                if (chunkRelPos.z == 15 && (neighbor = chunkManager::neighborPlusZ(chunkPos))) {

                    neighbor->setBlockNeighbor(chunkRelPos.x, chunkRelPos.y, blockViewDir::NEGZ, block::emptyBlock());
                    chunkManager::issueChunkMeshJob(neighbor, chunkJobType::PRIORITYREMESH);

                }

            }

        }

    }

    void player::placeSelectedBlock() {

        if (!GUImanager::levelGUIOpened()) {

            float xOld = std::floor(oldSelectedBlockPos_.x),
                  yOld = std::floor(oldSelectedBlockPos_.y),
                  zOld = std::floor(oldSelectedBlockPos_.z),
                  x = std::floor(selectedBlockPos_.x),
                  y = std::floor(selectedBlockPos_.y),
                  z = std::floor(selectedBlockPos_.z);
            bool isSolid = !blockToPlace_.load()->isEmptyBlock();

            // Only one coordinate may differ between the two positions.
            if (xOld != x) {

                if (yOld != y)
                    yOld = y;

                if (zOld != z)
                    zOld = z;

            }
            else if (yOld != y) { // xOld == x

                if (zOld != z)
                    zOld = z;

            } // else xOld == x && yOld == y

            chunk* selectedChunk = chunkManager::selectChunkByChunkPos(xOld, yOld, zOld);
            if (selectedChunk && chunkManager::isEmptyBlock(xOld, yOld, zOld) && !selectedBlock_->isEmptyBlock()) {

                chunk* neighbor = nullptr;

                const vec3& chunkPos = selectedChunk->chunkPos();
                vec3 chunkRelPos{ floorMod(xOld, SCX),
                                  floorMod(yOld, SCY),
                                  floorMod(zOld, SCZ) };

                selectedChunk->setBlock(chunkRelPos, *blockToPlace_);

                chunkManager::issueChunkMeshJob(selectedChunk, chunkJobType::PRIORITYREMESH);

                if (chunkRelPos.x == 0 && (neighbor = chunkManager::neighborMinusX(chunkPos))) {
                
                    neighbor->setBlockNeighbor(chunkRelPos.y, chunkRelPos.z, blockViewDir::PLUSX, *blockToPlace_);
                    chunkManager::issueChunkMeshJob(neighbor, chunkJobType::PRIORITYREMESH);
                
                }

                if (chunkRelPos.x == 15 && (neighbor = chunkManager::neighborPlusX(chunkPos))) {

                    neighbor->setBlockNeighbor(chunkRelPos.y, chunkRelPos.z, blockViewDir::NEGX, *blockToPlace_);
                    chunkManager::issueChunkMeshJob(neighbor, chunkJobType::PRIORITYREMESH);

                }

                if (chunkRelPos.y == 0 && (neighbor = chunkManager::neighborMinusY(chunkPos))) {

                    neighbor->setBlockNeighbor(chunkRelPos.x, chunkRelPos.z, blockViewDir::PLUSY, *blockToPlace_);
                    chunkManager::issueChunkMeshJob(neighbor, chunkJobType::PRIORITYREMESH);

                }

                if (chunkRelPos.y == 15 && (neighbor = chunkManager::neighborPlusY(chunkPos))) {

                    neighbor->setBlockNeighbor(chunkRelPos.x, chunkRelPos.z, blockViewDir::NEGY, *blockToPlace_);
                    chunkManager::issueChunkMeshJob(neighbor, chunkJobType::PRIORITYREMESH);

                }

                if (chunkRelPos.z == 0 && (neighbor = chunkManager::neighborMinusZ(chunkPos))) {

                    neighbor->setBlockNeighbor(chunkRelPos.x, chunkRelPos.y, blockViewDir::PLUSZ, *blockToPlace_);
                    chunkManager::issueChunkMeshJob(neighbor, chunkJobType::PRIORITYREMESH);

                }

                if (chunkRelPos.z == 15 && (neighbor = chunkManager::neighborPlusZ(chunkPos))) {

                    neighbor->setBlockNeighbor(chunkRelPos.x, chunkRelPos.y, blockViewDir::NEGZ, *blockToPlace_);
                    chunkManager::issueChunkMeshJob(neighbor, chunkJobType::PRIORITYREMESH);

                }

            }

        }

    }

    void player::processSelectionRaycast() {

        while (game::threadsExecute[0]) {

            selectBlock();

            if (destroyBlock_) {

                destroySelectedBlock();
                destroyBlock_ = false;

            }
            else if (placeBlock_) {

                placeSelectedBlock();
                placeBlock_ = false;

            }

            {

                using namespace std::chrono_literals;

                std::this_thread::sleep_for(1ms);

            }

        }

    }

    void player::setBlockToPlace(block& block) {

        if (block != *blockToPlace_) {

            blockToPlace_ = &block;

            GUIelement& element = GUImanager::getGUIElement("blockPreview");
            element.lockMutex();
            element.changeTextureID(block.textureID());
            element.unlockMutex();

        }

    }

    void player::changeTransform(const vec3& newPos, const vec3& newRot) {

        playerTransform_->position = newPos;
        playerTransform_->rotation = newRot;

    }

    void player::updateTransform(float timeStep) {

        // Camera's movement.
        if (moveNorth_) {

            playerTransform_->position += playerTransform_->viewDirection * movementSpeed_ * timeStep;
            //transform_.position += forwardAxis_ * movementSpeed_ * timeStep;
            moveNorth_ = false;

        }

        if (moveSouth_) {

            playerTransform_->position -= playerTransform_->viewDirection * movementSpeed_ * timeStep;
            //transform_.position -= forwardAxis_ * movementSpeed_ * timeStep;
            moveSouth_ = false;

        }

        if (moveEast_) {

            playerTransform_->position -= playerTransform_->Zaxis * movementSpeed_ * timeStep;
            moveEast_ = false;

        }

        if (moveWest_) {

            playerTransform_->position += playerTransform_->Zaxis * movementSpeed_ * timeStep;
            moveWest_ = false;

        }

        if (moveUp_) {

            playerTransform_->position += playerTransform_->Yaxis * movementSpeed_ * timeStep;
            moveUp_ = false;

        }

        if (moveDown_) {

            playerTransform_->position -= playerTransform_->Yaxis * movementSpeed_ * timeStep;
            moveDown_ = false;

        }

        if (rollRight_) {

            //roll_ -= rollSpeed_ * timeStep;
            rollRight_ = false;

        }

        if (rollLeft_) {

            //roll_ += rollSpeed_ * timeStep;
            rollLeft_ = false;

        }

        // Update chunk-relative coordinates.
        playerTransform_->chunkPosition.x = trunc(playerTransform_->position.x / SCX);
        playerTransform_->chunkPosition.y = trunc(playerTransform_->position.y / SCY);
        playerTransform_->chunkPosition.z = trunc(playerTransform_->position.z / SCZ);

        // Get and process mouse input.
        oldMouseX_ = mouseX_;
        oldMouseY_ = mouseY_;

        #if GRAPHICS_API == OPENGL

            glfwGetCursorPos(window_->windowAPIpointer(), &mouseX_, &mouseY_);

        #else

        #endif

        pitchViewDir_ += (oldMouseY_ - mouseY_) * mouseSensibility_; // LO QUE HAY QUE GUARDAR DE HACIA DONDE MIRA EL PLAYER ES ESTO.
        yawViewDir_ += (oldMouseX_ - mouseX_) * mouseSensibility_; // GUARDA EL OLD MOUSEY, MOUSEY, OLD MOUSEX Y MOUSEX Y HAZ glfwSetCursorPos

        if (pitchViewDir_ > 89.0f)
            pitchViewDir_ = 89.0f;
        else if (pitchViewDir_ < -89.0f)
            pitchViewDir_ = -89.0f;

        // Obtain the angles needed to rotate a vector equal to vec3FixedUp to the current value of the transform's Y axis.
        pitchAxis_ = vec3FixedEast;
        yawAxis_ = vec3FixedUp;
        rollAxis_ = vec3FixedNorth;

        playerTransform_->gravityDirection = vec3(0.0f, -1.0f, 0.0f);

        playerTransform_->Yaxis = -glm::normalize(playerTransform_->gravityDirection);
        playerTransform_->Xaxis = vec3FixedNorth;

        playerTransform_->rotation.x = glm::degrees(glm::acos(glm::dot(playerTransform_->Yaxis, vec3FixedNorth))) - 90.0f;
        playerTransform_->rotation.y = glm::degrees(glm::acos(glm::dot(playerTransform_->Yaxis, vec3FixedUp)));
        playerTransform_->rotation.z = glm::degrees(glm::acos(glm::dot(playerTransform_->Yaxis, vec3FixedEast))) - 90.0f;

        // Change the orientation of the camera based on the actual gravity direction of the entity this camera is attached to.
        pitchAxis_ = glm::rotate(pitchAxis_, glm::radians(playerTransform_->rotation.y), rollAxis_);
        yawAxis_ = glm::rotate(yawAxis_, glm::radians(playerTransform_->rotation.y), rollAxis_);

        yawAxis_ = glm::rotate(yawAxis_, glm::radians(playerTransform_->rotation.x), pitchAxis_);
        rollAxis_ = glm::rotate(rollAxis_, glm::radians(playerTransform_->rotation.x), pitchAxis_);

        pitchAxis_ = glm::rotate(pitchAxis_, glm::radians(playerTransform_->rotation.z), yawAxis_);
        rollAxis_ = glm::rotate(rollAxis_, glm::radians(playerTransform_->rotation.z), yawAxis_);

        if (playerTransform_->Yaxis != vec3FixedUp && playerTransform_->Yaxis != vec3FixedDown)
            playerTransform_->Xaxis = glm::cross(yawAxis_, playerTransform_->Yaxis);
        else // Particular case when glm::cross will return (0,0,0)
            playerTransform_->Xaxis = glm::rotate(playerTransform_->Xaxis, glm::radians(playerTransform_->rotation.y), rollAxis_);

        playerTransform_->viewDirection = quaternion::rotateVector(playerTransform_->Xaxis, yawViewDir_, playerTransform_->Yaxis);
        playerTransform_->Zaxis = glm::cross(playerTransform_->viewDirection, playerTransform_->Yaxis);
        playerTransform_->viewDirection = quaternion::rotateVector(playerTransform_->viewDirection, pitchViewDir_, playerTransform_->Zaxis);

        playerTransform_->Xaxis = glm::normalize(playerTransform_->Xaxis);
        playerTransform_->Zaxis = glm::normalize(playerTransform_->Zaxis);
        playerTransform_->viewDirection = glm::normalize(playerTransform_->viewDirection);

        camera_->updateTransform(playerTransform_);
    
    }

    void player::reset() {

        if (camera_) {

            delete camera_;
            camera_ = nullptr;

        }

        initialised_ = false;

        selectedBlock_ = nullptr;
        blockToPlace_ = nullptr;

        playerEntity_ = nullptr;
        playerTransform_ = nullptr;

    }

}