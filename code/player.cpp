#include "player.h"

#include "game.h"
#include "gui.h"

namespace VoxelEng {

    // 'player' class.

    bool player::initialised_ = false;
    GLFWwindow* player::window_ = nullptr;
    camera* player::camera_ = nullptr;
    float player::blockReachRange_ = 0.0f,
          player::blockSearchIncrement_ = 0.0f;
    const block* player::selectedBlock_ = nullptr;
    std::atomic<const block*> player::blockToPlace_ = nullptr;
    vec3 player::selectedBlockPos_ = vec3Zero,
         player::oldSelectedBlockPos_ = vec3Zero;
    std::atomic<bool> player::destroyBlock_ = false,
                      player::placeBlock_ = false;


    void player::init(float FOV, float zNear, float zFar, window& window, unsigned int blockReachRange) {

        if (initialised_)
            logger::errorLog("Player system is already initialised");
        else {

            if (game::selectedEngineMode() == engineMode::AIMENULOOP) {

                window_ = window.windowAPIpointer();
                camera_ = new camera(FOV, zNear, zFar, window, true);
                blockReachRange_ = blockReachRange;
                blockSearchIncrement_ = 0.1f;
                selectedBlock_ = block::emptyBlockP();
                blockToPlace_ = block::emptyBlockP(); // TODO. ADD PROPER PLAYER MODEL.
                playerEntity_ = &entityManager::getEntity(entityManager::registerEntity(2, vec3Zero, vec3Zero, nullptr));

                // NEXT. GET ALL THE PLAYERENTITY'S VARIABLE SYNCHRONIZED (WITH THE SAME VALUE) AS THE PLAYER'S AND SEE IF THE MODEL RENDERS PROPERLY AND IF NOT FIX IT

            }
            else
                logger::errorLog("The player class must be initialised in the AI menu loop");

        }

    }

    void player::selectBlock() {

        float step = blockSearchIncrement_;
        const vec3& dir = camera_->viewDirection(),
            pos = camera_->pos();
        vec3 blockPos;
        selectedBlock_ = block::emptyBlockP();

        while (step < blockReachRange_ && selectedBlock_->isEmptyBlock()) {

            selectedBlockPos_ = pos + (dir * step);

            blockPos.x = floor(selectedBlockPos_.x);
            blockPos.y = floor(selectedBlockPos_.y);
            blockPos.z = floor(selectedBlockPos_.z);

            selectedBlock_ = (chunkManager::getChunkLoadLevel(chunkManager::getChunkCoords(blockPos)) == VoxelEng::chunkLoadLevel::DECORATED) ? &chunkManager::getBlock(blockPos) : block::emptyBlockP();

            if (selectedBlock_->isEmptyBlock()) { // No non-empty block found. Continue searching.

                oldSelectedBlockPos_ = selectedBlockPos_;

                step += blockSearchIncrement_;

            }
            else {

                float dummy = 2.50f;

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

                vec3 chunkRelPos = chunkManager::getChunkRelCoords(selectedBlockPos_);

                selectedChunk->setBlock(chunkRelPos, block::emptyBlock());

                chunkManager::issueChunkMeshJob(selectedChunk, false, true);

                if (chunkRelPos.x == 0 && (neighbor = chunkManager::neighborMinusX(selectedChunk->chunkPos())))
                    chunkManager::issueChunkMeshJob(neighbor, false, true);

                if (chunkRelPos.x == 15 && (neighbor = chunkManager::neighborPlusX(selectedChunk->chunkPos())))
                    chunkManager::issueChunkMeshJob(neighbor, false, true);

                if (chunkRelPos.y == 0 && (neighbor = chunkManager::neighborMinusY(selectedChunk->chunkPos())))
                    chunkManager::issueChunkMeshJob(neighbor, false, true);

                if (chunkRelPos.y == 15 && (neighbor = chunkManager::neighborPlusY(selectedChunk->chunkPos())))
                    chunkManager::issueChunkMeshJob(neighbor, false, true);

                if (chunkRelPos.z == 0 && (neighbor = chunkManager::neighborMinusZ(selectedChunk->chunkPos())))
                    chunkManager::issueChunkMeshJob(neighbor, false, true);

                if (chunkRelPos.z == 15 && (neighbor = chunkManager::neighborPlusZ(selectedChunk->chunkPos())))
                    chunkManager::issueChunkMeshJob(neighbor, false, true);

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

                vec3 chunkRelPos{ floorMod(xOld, SCX),
                                   floorMod(yOld, SCY),
                                   floorMod(zOld, SCZ) };

                selectedChunk->setBlock(chunkRelPos, *blockToPlace_);

                chunkManager::issueChunkMeshJob(selectedChunk, false, true);

                if (chunkRelPos.x == 0 && (neighbor = chunkManager::neighborMinusX(selectedChunk->chunkPos())))
                    chunkManager::issueChunkMeshJob(neighbor, false, true);

                if (chunkRelPos.x == 15 && (neighbor = chunkManager::neighborPlusX(selectedChunk->chunkPos())))
                    chunkManager::issueChunkMeshJob(neighbor, false, true);

                if (chunkRelPos.y == 0 && (neighbor = chunkManager::neighborMinusY(selectedChunk->chunkPos())))
                    chunkManager::issueChunkMeshJob(neighbor, false, true);

                if (chunkRelPos.y == 15 && (neighbor = chunkManager::neighborPlusY(selectedChunk->chunkPos())))
                    chunkManager::issueChunkMeshJob(neighbor, false, true);

                if (chunkRelPos.z == 0 && (neighbor = chunkManager::neighborMinusZ(selectedChunk->chunkPos())))
                    chunkManager::issueChunkMeshJob(neighbor, false, true);

                if (chunkRelPos.z == 15 && (neighbor = chunkManager::neighborPlusZ(selectedChunk->chunkPos())))
                    chunkManager::issueChunkMeshJob(neighbor, false, true);

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

    void player::changeTransform(float newX, float newY, float newZ, float pitch, float yaw, float roll) {

        camera_->setPos(newX, newY, newZ);
        camera_->setChunkPos(chunkManager::getChunkCoords(newX, newY, newZ));
        camera_->rotation(pitch, yaw, roll);

        playerEntity_->x() = newX;
        playerEntity_->y() = newY;
        playerEntity_->z() = newZ;
        playerEntity_->rotateViewPitch(pitch);
        playerEntity_->rotateViewYaw(yaw);
        playerEntity_->rotateViewRoll(roll);

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