#include "entity.h"
#include <cmath>
#include <cstddef>
#include <string>
#include "block.h"
#include "camera.h"
#include "game.h"
#include "graphics.h"
#include "gui.h"
#include "logger.h"
#include "utilities.h"
#include "worldGen.h"

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
    entity* player::playerEntity_ = nullptr;


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
            
                chunk * neighbor = nullptr;

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

    void player::cleanUp() {

        if (camera_) {
        
            delete camera_;
            camera_ = nullptr;
        
        }
       
        initialised_ = false;

        selectedBlock_ = nullptr;
        blockToPlace_ = nullptr;

    }


    // 'entity' class.

    const float entity::piDiv = 3.1415926f / 180.0f;


    entity::entity(unsigned int modelID, const vec3& pos, const vec3& rot, tickFunc func)
        : model_(&models::getModelAt(modelID)), pos_(pos), rot_(rot),
        updateXRotation_(false), updateYRotation_(false), updateZRotation_(false),
        tickFunc_(func) {
    
        rotate(rot);
    
    }

    void entity::rotate(float x, float y, float z) {
    
        if (x != 0) {
        
            rot_.x += x;
            updateXRotation_ = true;
        
        }

        if (y != 0) {

            rot_.y += y;
            updateYRotation_ = true;

        }

        if (z != 0) {

            rot_.z += z;
            updateZRotation_ = true;

        }

    }

    void entity::rotateX(float angle) {

        if (angle) {

            rot_.x += angle;
            updateXRotation_ = true;

        }

    }

    void entity::rotateY(float angle) {

        if (angle) {

            rot_.y += angle;
            updateYRotation_ = true;

        }

    }

    void entity::rotateZ(float angle) {

        if (angle) {

            rot_.z += angle;
            updateZRotation_ = true;

        }

    }

    void entity::rotateView(float roll, float pitch, float yaw) {

        if (roll != 0) {

            rot_.z += roll;
            updateZRotation_ = true;

        }

        if (pitch != 0) {

            rot_.x += pitch;
            updateXRotation_ = true;

        }

        if (yaw != 0) {

            rot_.y += yaw;
            updateYRotation_ = true;

        }

    }

    void entity::rotateViewRoll(float angle) {
    
        if (angle != 0) {

            rot_.z += angle;
            updateZRotation_ = true;

        }
    
    }

    void entity::rotateViewPitch(float angle) {
    
        if (angle != 0) {

            rot_.x += angle;
            updateXRotation_ = true;

        }
    
    }

    void entity::rotateViewYaw(float angle) {
    
        if (angle != 0) {

            rot_.y += angle;
            updateYRotation_ = true;

        }
    
    }


    // 'entityManager' class.

    bool entityManager::initialised_ = false,
         entityManager::firstManagementIteration_ = true;
    std::vector<entity> entityManager::entities_;
    std::vector<batch> entityManager::batches_;
    std::vector<model>* entityManager::renderingDataWrite_ = nullptr,
                      * entityManager::renderingDataRead_ = nullptr;
    std::unordered_set<unsigned int> entityManager::activeEntityID_,
                                     entityManager::activeBatchID_,
                                     entityManager::freeEntityID_,
                                     entityManager::freeBatchID_,
                                     entityManager::inactiveEntityID_,
                                     entityManager::inactiveBatchID_,
                                     entityManager::deleteableEntityID_;
    std::list<unsigned int> entityManager::tickingEntityID_;
    std::unordered_map<unsigned int, unsigned int> entityManager::entityBatch_;
    std::recursive_mutex entityManager::entitiesMutex_,
                         entityManager::batchesMutex_;
    std::condition_variable entityManager::entityManagerCV_;
    std::atomic<bool> entityManager::entityMngCVContinue_ = false;
    std::mutex entityManager::syncMutex_;
    std::unique_lock<std::mutex> entityManager::syncLock_(entityManager::syncMutex_, std::defer_lock);
    unsigned int entityManager::ticksPerFrame_ = 0;

    
    void entityManager::init() {
    
        if (initialised_)
            logger::errorLog("Entity management system is already initialised");
        else {
        
            firstManagementIteration_ = true;

            if (!game::AImodeON()) {
            
                if (!renderingDataWrite_)
                    renderingDataWrite_ = new std::vector<model>();
                if (!renderingDataRead_)
                    renderingDataRead_ = new std::vector<model>();
            
            }
            
            ticksPerFrame_ = 30;

            initialised_ = true;
        
        }
    
    }

    bool entityManager::isEntityRegistered(entityID entityID) {

        std::unique_lock<std::recursive_mutex> lockEntities(entitiesMutex_);

        return entityID < entities_.size() && freeEntityID_.find(entityID) == freeEntityID_.cend();

    }

    bool entityManager::isEntityActiveAt(entityID entityID) {

        if (isEntityRegistered(entityID))
            return isEntityActive(entityID);
        else
            logger::errorLog("Entity with ID " + std::to_string(entityID) + " was not found");

    }

    bool entityManager::isEntityActive(entityID entityID) {

        std::unique_lock<std::recursive_mutex> lockEntities(entitiesMutex_);

        return inactiveEntityID_.find(entityID) == inactiveEntityID_.cend();

    }

    unsigned int entityManager::registerBatch_() {

        std::unique_lock<std::recursive_mutex> lockBatches(batchesMutex_);

        batches_.emplace_back();

        return batches_.size() - 1;

    }

    void entityManager::setAImode(bool on) {
    
        if (on) {
        
            if (renderingDataWrite_) {

                delete renderingDataWrite_;
                renderingDataWrite_ = nullptr;

            }

            if (renderingDataRead_) {

                delete renderingDataRead_;
                renderingDataRead_ = nullptr;

            }
        
        }
        else {

            if (!renderingDataWrite_)
                renderingDataWrite_ = new std::vector<model>();

            if (!renderingDataRead_)
                renderingDataRead_ = new std::vector<model>();
        
        }
    
    }

    void entityManager::manageEntities() {

        firstManagementIteration_ = false;

        // Process active entities that have a corresponding tick function ...
        entitiesMutex_.lock();
        unsigned int ID;
        for (unsigned int i = 0; i < ticksPerFrame_ && i < tickingEntityID_.size(); i++) { // It processes min(ticksPerFrame_, tickingEntityID_.size()) ticks.

            ID = tickingEntityID_.front();
            entities_[ID].tickFunc_();
            tickingEntityID_.push_back(ID);
            tickingEntityID_.pop_front();

        }

        // ... and delete all unused entities.
        for (auto it = deleteableEntityID_.cbegin(); it != deleteableEntityID_.cend(); it++)
            deleteEntity(*it);
        deleteableEntityID_.clear();

        entitiesMutex_.unlock();


        // Regenerate all batches that need to be.
        batchesMutex_.lock();
        bool synchronise = false;
        for (unsigned int i = 0; i < batches_.size(); i++)
            if (batches_[i].isDirty()) {
                    
                if (renderingDataWrite_->size() <= i)
                    renderingDataWrite_->push_back(batches_[i].generateVertices());
                else
                    renderingDataWrite_->operator[](i) = batches_[i].generateVertices();
                synchronise = true;
                    
            }  

        batchesMutex_.unlock();

        // Sync with rendering thread if necessary to update the models being drawn.
        if (synchronise)
            entityManagerCV_.wait(syncLock_);

    }

    unsigned int entityManager::nEntities() {

        std::unique_lock<std::recursive_mutex> lock(entitiesMutex_);

        return entities_.size();

    }

    entityID entityManager::registerEntity(unsigned int modelID, float posX, float posY, float posZ, float rotX, float rotY, float rotZ, tickFunc func) {

        entityID entityID = 0,
                 batchID = 0;


        std::unique_lock<std::recursive_mutex> lockEntities(entitiesMutex_);

        // Get entity's ID and register it inside the 'entities_' structure.
        if (freeEntityID_.empty()) {

            entityID = entities_.size();
            entities_.emplace_back(modelID, vec3{ posX, posY, posZ }, vec3{ rotX, rotY, rotZ }, func);

        }
        else {

            entityID = *freeEntityID_.begin();
            freeEntityID_.erase(entityID);
            
            entities_[entityID].setModelID(modelID);
            entities_[entityID].pos_ = vec3{ posX, posY, posZ };
            entities_[entityID].rotate(rotX, rotY, rotZ);
            entities_[entityID].tickFunc_ = func;

        }
        activeEntityID_.insert(entityID);

        if (func)
            tickingEntityID_.push_back(entityID);

        if (!game::AImodeON()) { // Register the new entity inside a batch only if AI mode is disabled.
            
            std::unique_lock<std::recursive_mutex> lockBatches(batchesMutex_);

            if (batches_.empty()) { // If no batch is registered.

                batchID = registerBatch_();

                if (!batches_[batches_.size() - 1].addEntity(entityID))
                    logger::errorLog("Entity with ID: " + std::to_string(entityID) + " has a model too big for a batch!");

            }
            else {

                if (freeBatchID_.empty()) {

                    if (!batches_[batches_.size() - 1].addEntity(entityID)) { // If last created batch cannot store the entity's model, then create another batch.

                        batchID = registerBatch_();

                        if (!batches_[batches_.size() - 1].addEntity(entityID))
                            logger::errorLog("Entity with ID " + std::to_string(entityID) + " has a model with too many vertices for a batch.");

                    }
                    else
                        batchID = batches_.size() - 1;

                }
                else {

                    bool found = false;
                    auto it = freeBatchID_.cbegin();
                    for (it; it != freeBatchID_.cend() && !found;) // Check if new entity's model fits into one of the already created batches.
                        if (!(found = batches_[*it].addEntity(entityID)))
                            it++;

                    if (found) {

                        if (batches_[*it].size() == BATCH_MAX_VERTEX_COUNT)
                            freeBatchID_.erase(*it);

                        batchID = *it;

                    }
                    else {

                        batchID = registerBatch_();

                        if (!batches_[batches_.size() - 1].addEntity(entityID))
                            logger::errorLog("Entity with ID: " + std::to_string(entityID) + " has a model too big for a batch!");

                    }

                }

            }

            // Associate entity and corresponding batch.
            entityBatch_[entityID] = batchID;

        }

        return entityID;

    }

    entity& entityManager::getEntityAt(entityID entityID) {

        if (isEntityRegistered(entityID))
            return getEntity(entityID);
        else
            logger::errorLog("Entity with ID " + std::to_string(entityID) + " is not registered");
    
    }

    entity& entityManager::getEntity(entityID entityID) {

        std::unique_lock<std::recursive_mutex> lockEntities(entitiesMutex_);
        return entities_[entityID];

    }

    void entityManager::changeEntityActiveStateAt(entityID entityID, bool active) {

        if (isEntityRegistered(entityID)) {

            if (active && inactiveEntityID_.find(entityID) != inactiveEntityID_.cend()) {

                std::unique_lock<std::recursive_mutex> lockEntities(entitiesMutex_);

                inactiveEntityID_.erase(entityID);
                activeEntityID_.insert(entityID);

                if (entities_[entityID].tickFunc_)
                    tickingEntityID_.push_back(entityID);
                

            }
            else if(!active && activeEntityID_.find(entityID) != activeEntityID_.cend()) {

                std::unique_lock<std::recursive_mutex> lockEntities(entitiesMutex_);

                inactiveEntityID_.insert(entityID);
                activeEntityID_.erase(entityID);
                tickingEntityID_.remove(entityID);

            }

            // Reflect changes on respective batch.
            std::unique_lock<std::recursive_mutex> lockBatches(batchesMutex_);

            batches_[entityBatch_[entityID]].changeActiveState(entityID, active);
        
        }
        else
            logger::errorLog("Entity with ID " + std::to_string(entityID) + " was not found");

    }

    void entityManager::deleteEntityAt(entityID entityID) {

        if (isEntityRegistered(entityID))
            deleteEntity(entityID);
        else
            logger::errorLog("There is no entity with ID " + std::to_string(entityID) + "to erase");

    }

    void entityManager::deleteEntity(entityID entityID) {

        std::unique_lock<std::recursive_mutex> lockEntities(entitiesMutex_);

        freeEntityID_.insert(entityID);

        if (isEntityActive(entityID)) {
        
            activeEntityID_.erase(entityID);
            tickingEntityID_.remove(entityID);
        
        } 
        else
            inactiveEntityID_.erase(entityID);

        // Remove entity from its corresponding batch and
        // mark batch as free if no more entities are related to it.
        // Also reflect changes in said batch.
        if (!game::AImodeON()) {
        
            std::unique_lock<std::recursive_mutex> lockBatches(batchesMutex_);
            unsigned int batchID = entityBatch_[entityID];
            if (batches_[batchID].deleteEntity(entityID))
                deleteBatch_(batchID);

            entityBatch_.erase(entityID);
        
        }

        // Reset entity's attributes here.
        entities_[entityID].rot_ = vec3Zero;
        
    }

    void entityManager::swapReadWrite() {

        std::vector<model>* aux = renderingDataRead_;

        renderingDataRead_ = renderingDataWrite_;
        renderingDataWrite_ = aux;

    }

    void entityManager::moveEntity(entityID entityID, float x, float y, float z) {
    
        if (entityManager::isEntityRegistered(entityID)) {

            vec3& pos = entityManager::getEntity(entityID).pos();
            pos.x += x;
            pos.y += y;
            pos.z += z;

            if (y != 0)
                y = y - 1 + 1;

            if (!game::AImodeON())
                batches_[entityBatch_[entityID]].isDirty() = true;

        }
        else
            logger::errorLog("Entity with ID " + std::to_string(entityID) + " was not found");
    
    }

    void entityManager::clean() {

        std::unique_lock<std::recursive_mutex> lockBatches(batchesMutex_);
        std::unique_lock<std::recursive_mutex> lockEntities(entitiesMutex_);
        std::unique_lock<std::mutex> syncLock(syncMutex_);

        entities_.clear();

        batches_.clear();

        entityBatch_.clear();

        activeEntityID_.clear();

        activeBatchID_.clear();

        freeEntityID_.clear();

        freeBatchID_.clear();

        tickingEntityID_.clear();

        if (renderingDataWrite_)
            renderingDataWrite_->clear();

        if (renderingDataRead_)
            renderingDataRead_->clear();

    }

    void entityManager::cleanUp() {

        std::unique_lock<std::recursive_mutex> lockBatches(batchesMutex_);
        std::unique_lock<std::recursive_mutex> lockEntities(entitiesMutex_);
        std::unique_lock<std::mutex> syncLock(syncMutex_);

        entities_.clear();

        batches_.clear();

        entityBatch_.clear();

        activeEntityID_.clear();

        activeBatchID_.clear();

        freeEntityID_.clear();

        freeBatchID_.clear();

        tickingEntityID_.clear();

        if (renderingDataWrite_) {
        
            renderingDataWrite_->clear();
            renderingDataWrite_ = nullptr;
        
        }

        if (renderingDataRead_) {

            renderingDataRead_->clear();
            renderingDataRead_ = nullptr;

        }

        initialised_ = false;

    }

    bool entityManager::isBatchRegistered_(unsigned int batchID) {

        std::unique_lock<std::recursive_mutex> lockBatches(batchesMutex_);

        return batchID < batches_.size() && freeBatchID_.find(batchID) == freeBatchID_.cend();

    }

    bool entityManager::isBatchActiveAt_(unsigned int batchID) {

        if (isBatchRegistered_(batchID))
            return isBatchActive_(batchID);
         else
            logger::errorLog("Batch with ID " + std::to_string(batchID) + " was not found");

    }

    bool entityManager::isBatchActive_(unsigned int batchID) {

        std::unique_lock<std::recursive_mutex> lockBatches(batchesMutex_);
        return inactiveBatchID_.find(batchID) == inactiveBatchID_.cend();

    }

    void entityManager::changeBatchActiveStateAt_(unsigned int batchID, bool active) {
    
        if (isBatchRegistered_(batchID)) {

            changeBatchActiveState_(batchID, active);

        }
        else
            logger::errorLog("Batch with ID " + std::to_string(batchID) + " was not found");
    
    }

    void entityManager::changeBatchActiveState_(unsigned int batchID, bool active) {

        if (active) {

            std::unique_lock<std::recursive_mutex> lockBatches(batchesMutex_);

            inactiveBatchID_.erase(batchID);
            activeBatchID_.insert(batchID);

        }
        else {

            std::unique_lock<std::recursive_mutex> lockBatches(batchesMutex_);

            inactiveBatchID_.insert(batchID);
            activeBatchID_.erase(batchID);

        }

    }

    void entityManager::deleteBatchAt_(unsigned int batchID) {
    
        if (isBatchRegistered_(batchID))
            deleteBatch_(batchID);
        else
            logger::errorLog("No batch with ID " + std::to_string(batchID) + " was found");

    }

    void entityManager::deleteBatch_(unsigned int batchID) {

        std::unique_lock<std::recursive_mutex> lockBatches(batchesMutex_);

        freeBatchID_.insert(batchID);
        batches_[batchID].clear();
        if (isBatchActive_(batchID))
            activeBatchID_.erase(batchID);
        else
            inactiveBatchID_.erase(batchID);

    }

}