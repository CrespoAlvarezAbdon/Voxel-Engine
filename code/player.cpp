#include "player.h"

#include <game.h>
#include <gui.h>

namespace VoxelEng {

    // 'player' class.

    bool player::initialised_ = false;
    bool player::moveUp_ = false;
    bool player::moveDown_ = false;
    bool player::moveNorth_ = false;
    bool player::moveSouth_ = false;
    bool player::moveEast_ = false;
    bool player::moveWest_ = false;
    bool player::rollRight_ = false;
    bool player::rollLeft_ = false;
    bool player::firstTransformUpdate_ = false;
    window* player::window_ = nullptr;
    camera* player::camera_ = nullptr;
    float player::blockReachRange_ = 0.0f;
    float player::blockSearchIncrement_ = 0.0f;
    float player::movementSpeed_ = 0.0f;
    float player::rollSpeed_ = 0.0f;
    float player::pitchAngle_ = 0.0f;
    float player::yawAngle_ = 0.0f;
    float player::mouseSensibility_ = 0.0f;
    double player::mouseX_ = 0.0;
    double player::mouseY_ = 0.0;
    double player::oldMouseX_ = 0.0;
    double player::oldMouseY_ = 0.0;
    const block* player::selectedBlock_ = nullptr;
    std::atomic<const block*> player::blockToPlace_ = nullptr;
    vec3 player::selectedBlockPos_ = vec3Zero;
    vec3 player::oldSelectedBlockPos_ = vec3Zero;
    vec3 player::pitchAxis_ = vec3Zero;
    vec3 player::yawAxis_ = vec3Zero;
    vec3 player::rollAxis_ = vec3Zero;
    std::atomic<bool> player::destroyBlock_ = false;
    std::atomic<bool> player::placeBlock_ = false;
    transform player::playerTransform_;

    void player::init(float FOV, float zNear, float zFar, window& window, unsigned int blockReachRange) {

        if (initialised_)
            logger::errorLog("Player system is already initialised");
        else {

            if (game::selectedEngineMode() == engineMode::MENULOOP) {

                moveUp_ = false;
                moveDown_ = false;
                moveNorth_ = false;
                moveSouth_ = false;
                moveEast_ = false;
                moveWest_ = false;
                rollRight_ = false;
                rollLeft_ = false;
                firstTransformUpdate_ = true;
                window_ = &window;
                camera_ = new camera(FOV, zNear, zFar, window, true);
                blockReachRange_ = blockReachRange;
                blockSearchIncrement_ = 0.1f;
                movementSpeed_ = 10.0f;
                rollSpeed_ = 70.0f;
                pitchAngle_ = 0.0f;
                yawAngle_ = 0.0f;
                mouseSensibility_ = 0.25f;
                mouseX_ = 0.0;
                mouseY_ = 0.0;
                oldMouseX_ = 0.0;
                oldMouseY_ = 0.0;
                selectedBlock_ = block::emptyBlockP();
                blockToPlace_ = selectedBlock_;
                selectedBlockPos_ = vec3Zero;
                oldSelectedBlockPos_ = vec3Zero;
                pitchAxis_ = vec3Zero;
                yawAxis_ = vec3Zero;
                rollAxis_ = vec3Zero;
                destroyBlock_ = false;
                placeBlock_ = false;
                playerTransform_.position = vec3(0, 110, 0);

                initialised_ = true;

            }
            else
                logger::errorLog("The player class must be initialised in the AI menu loop");

        }

    }

    void player::selectBlock() {

        float step = blockSearchIncrement_;
        const vec3& dir = playerTransform_.viewDirection,
                    globalPos = camera_->globalPos();
        ivec3 blockPos;
        selectedBlock_ = block::emptyBlockP();

        while (step < blockReachRange_ && selectedBlock_->isEmptyBlock()) {

            selectedBlockPos_ = globalPos + (dir * step);

            blockPos.x = floor(selectedBlockPos_.x);
            blockPos.y = floor(selectedBlockPos_.y);
            blockPos.z = floor(selectedBlockPos_.z);

            selectedBlock_ = (chunkManager::getChunkLoadStatus(getChunkCoords(blockPos)) >= chunkLoadStatus::DECORATED) ? &chunkManager::getBlock(blockPos) : block::emptyBlockP();

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

            chunk* selectedChunk = chunkManager::getChunkByRealPos(selectedBlockPos_);

            if (selectedChunk && !selectedBlock_->isEmptyBlock()) {

                chunk* neighbor = nullptr;
                const ivec3& chunkPos = selectedChunk->chunkPos();
                ivec3 chunkRelPos = getChunkRelCoords(selectedBlockPos_);
                std::list<std::pair<chunkJobType, void*>> jobsData;

                const block& oldB = selectedChunk->setEmptyBlock(chunkRelPos);

                if (!selectedChunk->hasNoNeighborBlockLight(chunkRelPos) && blockToPlace_.load()->opacity() > blockOpacity::FULLTRANSPARENT) {

                    std::tuple<chunk*, std::list<ivec3>*>* data =
                        new std::tuple<chunk*, std::list<ivec3>*>(
                            selectedChunk, new std::list<ivec3>{ chunkRelPos });
                    jobsData.emplace_back(chunkJobType::NON_TRANSPARENT_BLOCK_REMOVED_ON_LIGHT, data);

                }

                if (oldB.emittedLight().isNull()) {

                    jobsData.emplace_back(chunkJobType::PRIORITYREMESH, selectedChunk);

                    if (chunkRelPos.x == 0 && (neighbor = chunkManager::neighborMinusX(chunkPos))) {

                        neighbor->setEmptyBlock(CHUNK_SIZE, chunkRelPos.y, chunkRelPos.z);
                        jobsData.emplace_back(chunkJobType::PRIORITYREMESH, neighbor);

                    }

                    if (chunkRelPos.x == 15 && (neighbor = chunkManager::neighborPlusX(chunkPos))) {

                        neighbor->setEmptyBlock(-1, chunkRelPos.y, chunkRelPos.z);
                        jobsData.emplace_back(chunkJobType::PRIORITYREMESH, neighbor);

                    }

                    if (chunkRelPos.y == 0 && (neighbor = chunkManager::neighborMinusY(chunkPos))) {

                        neighbor->setEmptyBlock(chunkRelPos.x, CHUNK_SIZE, chunkRelPos.z);
                        jobsData.emplace_back(chunkJobType::PRIORITYREMESH, neighbor);

                    }

                    if (chunkRelPos.y == 15 && (neighbor = chunkManager::neighborPlusY(chunkPos))) {

                        neighbor->setEmptyBlock(chunkRelPos.x, -1, chunkRelPos.z);
                        jobsData.emplace_back(chunkJobType::PRIORITYREMESH, neighbor);

                    }

                    if (chunkRelPos.z == 0 && (neighbor = chunkManager::neighborMinusZ(chunkPos))) {

                        neighbor->setEmptyBlock(chunkRelPos.x, chunkRelPos.y, CHUNK_SIZE);
                        jobsData.emplace_back(chunkJobType::PRIORITYREMESH, neighbor);

                    }

                    if (chunkRelPos.z == 15 && (neighbor = chunkManager::neighborPlusZ(chunkPos))) {

                        neighbor->setEmptyBlock(chunkRelPos.x, chunkRelPos.y, -1);
                        jobsData.emplace_back(chunkJobType::PRIORITYREMESH, neighbor);

                    }

                }
                else { // Old block had light.

                    if (chunkRelPos.x == 0 && (neighbor = chunkManager::neighborMinusX(chunkPos)))
                        neighbor->setEmptyBlock(CHUNK_SIZE, chunkRelPos.y, chunkRelPos.z);

                    if (chunkRelPos.x == 15 && (neighbor = chunkManager::neighborPlusX(chunkPos)))
                        neighbor->setEmptyBlock(-1, chunkRelPos.y, chunkRelPos.z);

                    if (chunkRelPos.y == 0 && (neighbor = chunkManager::neighborMinusY(chunkPos)))
                        neighbor->setEmptyBlock(chunkRelPos.x, CHUNK_SIZE, chunkRelPos.z);

                    if (chunkRelPos.y == 15 && (neighbor = chunkManager::neighborPlusY(chunkPos)))
                        neighbor->setEmptyBlock(chunkRelPos.x, -1, chunkRelPos.z);

                    if (chunkRelPos.z == 0 && (neighbor = chunkManager::neighborMinusZ(chunkPos)))
                        neighbor->setEmptyBlock(chunkRelPos.x, chunkRelPos.y, CHUNK_SIZE);

                    if (chunkRelPos.z == 15 && (neighbor = chunkManager::neighborPlusZ(chunkPos)))
                        neighbor->setEmptyBlock(chunkRelPos.x, chunkRelPos.y, -1);

                    std::tuple<chunk*, std::list<ivec3>*>* data = 
                        new std::tuple<chunk*, std::list<ivec3>*>(selectedChunk, new std::list<ivec3>{ chunkRelPos });
                    jobsData.emplace_back(chunkJobType::PRIORITYREMESH_REMOVEDLIGHT, data);
                
                }

                chunkManager::issueChunkJob(jobsData, false);

            }

        }

    }

    void player::placeSelectedBlock() {

        if (!GUImanager::levelGUIOpened()) {

            float xOld = std::floor(oldSelectedBlockPos_.x);
            float yOld = std::floor(oldSelectedBlockPos_.y);
            float zOld = std::floor(oldSelectedBlockPos_.z);
            float x = std::floor(selectedBlockPos_.x);
            float y = std::floor(selectedBlockPos_.y);
            float z = std::floor(selectedBlockPos_.z);
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

            chunk* selectedChunk = chunkManager::getChunkByRealPos(xOld, yOld, zOld);
            if (selectedChunk && chunkManager::isEmptyBlock(xOld, yOld, zOld) && !selectedBlock_->isEmptyBlock()) {

                chunk* neighbor = nullptr;
                const ivec3& chunkPos = selectedChunk->chunkPos();
                ivec3 chunkRelPos{ floorMod(xOld, CHUNK_SIZE),
                                   floorMod(yOld, CHUNK_SIZE),
                                   floorMod(zOld, CHUNK_SIZE) };
                std::list<std::pair<chunkJobType, void*>> jobsData;

                // If placed block blocks some light that was previously there, recalculate lights.
                if (!selectedChunk->getBlockLight(chunkRelPos).isZero() && blockToPlace_.load()->opacity() > blockOpacity::FULLTRANSPARENT) {

                    std::tuple<chunk*, std::list<ivec3>*, const block*>* data =
                        new std::tuple<chunk*, std::list<ivec3>*, const block*>(
                            selectedChunk, new std::list<ivec3>{ chunkRelPos }, blockToPlace_);
                    jobsData.emplace_back(chunkJobType::NON_TRANSPARENT_BLOCK_PLACED_ON_LIGHT, data);

                }

                // If placed block doesn't propagate new light, just remesh the chunk.
                if (blockToPlace_.load()->emittedLight().isNull()) {

                    jobsData.emplace_back(chunkJobType::PRIORITYREMESH, selectedChunk);

                    // MAÑANA. FACTORIZE THE NEIGHBOR SET BLOCK INSIDE THE PRIORITYREMESH 
                    // AND MAKE THE PRIORITY REMESH OF ADDED AND DELETE LIGHTS BE A TASK AT THE END OF ITS CORRESPONDING JOB.
                    if (chunkRelPos.x == 0 && (neighbor = chunkManager::neighborMinusX(chunkPos))) {

                        neighbor->setBlock(CHUNK_SIZE, chunkRelPos.y, chunkRelPos.z, *blockToPlace_);
                        jobsData.emplace_back(chunkJobType::PRIORITYREMESH, neighbor);

                    }

                    if (chunkRelPos.x == 15 && (neighbor = chunkManager::neighborPlusX(chunkPos))) {

                        neighbor->setBlock(-1, chunkRelPos.y, chunkRelPos.z, *blockToPlace_);
                        jobsData.emplace_back(chunkJobType::PRIORITYREMESH, neighbor);

                    }

                    if (chunkRelPos.y == 0 && (neighbor = chunkManager::neighborMinusY(chunkPos))) {

                        neighbor->setBlock(chunkRelPos.x, CHUNK_SIZE, chunkRelPos.z, *blockToPlace_);
                        jobsData.emplace_back(chunkJobType::PRIORITYREMESH, neighbor);

                    }

                    if (chunkRelPos.y == 15 && (neighbor = chunkManager::neighborPlusY(chunkPos))) {

                        neighbor->setBlock(chunkRelPos.x, -1, chunkRelPos.z, *blockToPlace_);
                        jobsData.emplace_back(chunkJobType::PRIORITYREMESH, neighbor);

                    }

                    if (chunkRelPos.z == 0 && (neighbor = chunkManager::neighborMinusZ(chunkPos))) {

                        neighbor->setBlock(chunkRelPos.x, chunkRelPos.y, CHUNK_SIZE, *blockToPlace_);
                        jobsData.emplace_back(chunkJobType::PRIORITYREMESH, neighbor);

                    }

                    if (chunkRelPos.z == 15 && (neighbor = chunkManager::neighborPlusZ(chunkPos))) {

                        neighbor->setBlock(chunkRelPos.x, chunkRelPos.y, -1, *blockToPlace_);
                        jobsData.emplace_back(chunkJobType::PRIORITYREMESH, neighbor);

                    }

                }
                else { // If placed block to place provides light, propagate said light.

                    if (chunkRelPos.x == 0 && (neighbor = chunkManager::neighborMinusX(chunkPos)))
                        neighbor->setBlock(CHUNK_SIZE, chunkRelPos.y, chunkRelPos.z, *blockToPlace_);

                    if (chunkRelPos.x == 15 && (neighbor = chunkManager::neighborPlusX(chunkPos)))
                        neighbor->setBlock(-1, chunkRelPos.y, chunkRelPos.z, *blockToPlace_);

                    if (chunkRelPos.y == 0 && (neighbor = chunkManager::neighborMinusY(chunkPos)))
                        neighbor->setBlock(chunkRelPos.x, CHUNK_SIZE, chunkRelPos.z, *blockToPlace_);

                    if (chunkRelPos.y == 15 && (neighbor = chunkManager::neighborPlusY(chunkPos)))
                        neighbor->setBlock(chunkRelPos.x, -1, chunkRelPos.z, *blockToPlace_);

                    if (chunkRelPos.z == 0 && (neighbor = chunkManager::neighborMinusZ(chunkPos)))
                        neighbor->setBlock(chunkRelPos.x, chunkRelPos.y, CHUNK_SIZE, *blockToPlace_);

                    if (chunkRelPos.z == 15 && (neighbor = chunkManager::neighborPlusZ(chunkPos)))
                        neighbor->setBlock(chunkRelPos.x, chunkRelPos.y, -1, *blockToPlace_);

                    jobsData.emplace_back(chunkJobType::PRIORITYREMESH_ADDEDLIGHT, selectedChunk);

                }

                const block& oldB = selectedChunk->setBlock(chunkRelPos, *blockToPlace_, true);
                chunkManager::issueChunkJob(jobsData, false);

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
            element.changeTextureID(block.textureID("faceY+"));
            element.unlockMutex();

        }

    }

    void player::updateTransform(float timeStep) {

        // Camera's movement.
        if (moveNorth_) {

            playerTransform_.position += playerTransform_.viewDirection * movementSpeed_ * timeStep;
            //transform_.position += forwardAxis_ * movementSpeed_ * timeStep;
            moveNorth_ = false;

        }

        if (moveSouth_) {

            playerTransform_.position -= playerTransform_.viewDirection * movementSpeed_ * timeStep;
            //transform_.position -= forwardAxis_ * movementSpeed_ * timeStep;
            moveSouth_ = false;

        }

        if (moveEast_) {

            playerTransform_.position -= playerTransform_.Zaxis * movementSpeed_ * timeStep;
            moveEast_ = false;

        }

        if (moveWest_) {

            playerTransform_.position += playerTransform_.Zaxis * movementSpeed_ * timeStep;
            moveWest_ = false;

        }

        if (moveUp_) {

            playerTransform_.position += playerTransform_.Yaxis * movementSpeed_ * timeStep;
            moveUp_ = false;

        }

        if (moveDown_) {

            playerTransform_.position -= playerTransform_.Yaxis * movementSpeed_ * timeStep;
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
        playerTransform_.chunkPosition.x = std::floor(playerTransform_.position.x / CHUNK_SIZE);
        playerTransform_.chunkPosition.y = std::floor(playerTransform_.position.y / CHUNK_SIZE);
        playerTransform_.chunkPosition.z = std::floor(playerTransform_.position.z / CHUNK_SIZE);

        // Get and process mouse input.
        oldMouseX_ = mouseX_;
        oldMouseY_ = mouseY_;

        #if GRAPHICS_API == OPENGL

            glfwGetCursorPos(window_->windowAPIpointer(), &mouseX_, &mouseY_);

        #else

        #endif

        if (firstTransformUpdate_) {
        
            oldMouseX_ = mouseX_;
            oldMouseY_ = mouseY_;
            firstTransformUpdate_ = false;
        
        }

        pitchAngle_ += (oldMouseY_ - mouseY_) * mouseSensibility_;
        yawAngle_ += (oldMouseX_ - mouseX_) * mouseSensibility_;

        if (pitchAngle_ > 89.0f)
            pitchAngle_ = 89.0f;
        else if (pitchAngle_ < -89.0f)
            pitchAngle_ = -89.0f;

        // Obtain the angles needed to rotate a vector equal to vec3FixedUp to the current value of the transform's Y axis.
        pitchAxis_ = vec3FixedEast;
        yawAxis_ = vec3FixedUp;
        rollAxis_ = vec3FixedNorth;

        playerTransform_.gravityDirection = vec3(0.0f, -1.0f, 0.0f);

        // This determines the camera's roll so that the camera is always standing
        // 'up' depending on the current gravity direction.
        playerTransform_.Yaxis = -glm::normalize(playerTransform_.gravityDirection); 

        playerTransform_.Xaxis = vec3FixedNorth;

        playerTransform_.rotation.x = glm::degrees(glm::acos(glm::dot(playerTransform_.Yaxis, vec3FixedNorth))) - 90.0f;
        playerTransform_.rotation.y = glm::degrees(glm::acos(glm::dot(playerTransform_.Yaxis, vec3FixedUp)));
        playerTransform_.rotation.z = glm::degrees(glm::acos(glm::dot(playerTransform_.Yaxis, vec3FixedEast))) - 90.0f;

        // Change the orientation of the camera based on the actual gravity direction of the entity this camera is attached to.
        pitchAxis_ = glm::rotate(pitchAxis_, glm::radians(playerTransform_.rotation.y), rollAxis_);
        yawAxis_ = glm::rotate(yawAxis_, glm::radians(playerTransform_.rotation.y), rollAxis_);

        yawAxis_ = glm::rotate(yawAxis_, glm::radians(playerTransform_.rotation.x), pitchAxis_);
        rollAxis_ = glm::rotate(rollAxis_, glm::radians(playerTransform_.rotation.x), pitchAxis_);

        pitchAxis_ = glm::rotate(pitchAxis_, glm::radians(playerTransform_.rotation.z), yawAxis_);
        rollAxis_ = glm::rotate(rollAxis_, glm::radians(playerTransform_.rotation.z), yawAxis_);

        if (playerTransform_.Yaxis != vec3FixedUp && playerTransform_.Yaxis != vec3FixedDown)
            playerTransform_.Xaxis = glm::cross(yawAxis_, playerTransform_.Yaxis);
        else // Particular case when glm::cross will return (0,0,0)
            playerTransform_.Xaxis = glm::rotate(playerTransform_.Xaxis, glm::radians(playerTransform_.rotation.y), rollAxis_);

        // Final result.
        playerTransform_.viewDirection = quaternion::rotateVector(playerTransform_.Xaxis, yawAngle_, playerTransform_.Yaxis);
        playerTransform_.Zaxis = glm::cross(playerTransform_.viewDirection, playerTransform_.Yaxis);
        playerTransform_.viewDirection = quaternion::rotateVector(playerTransform_.viewDirection, pitchAngle_, playerTransform_.Zaxis);

        // Normalize to avoid strange stuff.
        playerTransform_.Xaxis = glm::normalize(playerTransform_.Xaxis);
        playerTransform_.Zaxis = glm::normalize(playerTransform_.Zaxis);
        playerTransform_.viewDirection = glm::normalize(playerTransform_.viewDirection);

        camera_->updateTransform(playerTransform_);
        //entityManager::setTransform(playerEntity_->ID(), *playerTransform_); // TODO. MAKE PLAYER ENTITY MOVE WITH ENTITY
    
    }

    void player::reset() {

        if (camera_) {

            delete camera_;
            camera_ = nullptr;

        }

        initialised_ = false;

        selectedBlock_ = nullptr;
        blockToPlace_ = nullptr;

    }

}