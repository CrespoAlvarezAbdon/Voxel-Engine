#include <string>
#include <cmath>
#include <cstddef>
#include "entity.h"
#include "graphics.h"
#include "utilities.h"
#include "gui.h"
#include "logger.h"
#include "game.h"
#include "entity.h"
#include "worldGen.h"


namespace VoxelEng {

    // 'player' class.

    bool player::initialised_ = false;
    GLFWwindow* player::window_ = nullptr;
    camera* player::camera_ = nullptr;
    float player::blockReachRange_ = 0.0f,
          player::blockSearchIncrement_ = 0.0f;
    block player::selectedBlock_ = 0,
          player::blockToPlace_ = 0;
    vec3 player::selectedBlockPos_ = vec3Zero,
         player::oldSelectedBlockPos_ = vec3Zero;
    std::atomic<bool> player::destroyBlock_ = false,
                      player::placeBlock_ = false;


    void player::init(float FOV, float zNear, float zFar, window& window, unsigned int blockReachRange) {

        if (initialised_)
            logger::errorLog("Player system is already initialised");
        else {
        
            if (game::loopSelection() != GRAPHICALMENU)
                logger::errorLog("The engine is not in the graphical main menu loop");
            else {

                window_ = window.windowAPIpointer();
                camera_ = new camera(FOV, zNear, zFar, window, true);
                blockReachRange_ = blockReachRange;
                blockSearchIncrement_ = 0.10f;

            }
        
        }
            
    }

    void player::selectBlock() {

        float step = blockSearchIncrement_;
        const vec3& dir = camera_->direction(),
                    pos = camera_->pos();
        vec3 blockPos;
        selectedBlock_ = 0;

        while (step < blockReachRange_ && !selectedBlock_) {

            selectedBlockPos_ = pos + (dir * step);

            blockPos.x = floor(selectedBlockPos_.x);
            blockPos.y = floor(selectedBlockPos_.y);
            blockPos.z = floor(selectedBlockPos_.z);

            selectedBlock_ = (chunkManager::getChunkLoadLevel(chunkManager::getChunkCoords(blockPos)) == VoxelEng::chunkLoadLevel::DECORATED) ? chunkManager::getBlock(blockPos) : 0;

            if (!selectedBlock_) { // No non-null block found. Continue searching.
            
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

        if (!GUIManager::levelGUIOpened()) {

            std::unique_lock<std::recursive_mutex> lock(chunkManager::chunksMutex());

            chunk* selectedChunk = chunkManager::selectChunkByRealPos(selectedBlockPos_),
                 * neighbor = nullptr;

            if (selectedChunk && selectedBlock_) {

                vec3 chunkRelPos = chunkManager::getChunkRelCoords(selectedBlockPos_);

                selectedChunk->setBlock(chunkRelPos, 0);

                chunkManager::highPriorityUpdate(selectedChunk->chunkPos());

                if (chunkRelPos.x == 0 && (neighbor = chunkManager::neighborMinusX(selectedChunk->chunkPos())))
                    chunkManager::highPriorityUpdate(neighbor->chunkPos());

                if (chunkRelPos.x == 15 && (neighbor = chunkManager::neighborPlusX(selectedChunk->chunkPos())))
                    chunkManager::highPriorityUpdate(neighbor->chunkPos());

                if (chunkRelPos.y == 0 && (neighbor = chunkManager::neighborMinusY(selectedChunk->chunkPos())))
                    chunkManager::highPriorityUpdate(neighbor->chunkPos());

                if (chunkRelPos.y == 15 && (neighbor = chunkManager::neighborPlusY(selectedChunk->chunkPos())))
                    chunkManager::highPriorityUpdate(neighbor->chunkPos());

                if (chunkRelPos.z == 0 && (neighbor = chunkManager::neighborMinusZ(selectedChunk->chunkPos())))
                    chunkManager::highPriorityUpdate(neighbor->chunkPos());

                if (chunkRelPos.z == 15 && (neighbor = chunkManager::neighborPlusZ(selectedChunk->chunkPos())))
                    chunkManager::highPriorityUpdate(neighbor->chunkPos());

            }

        }

    }

    void player::placeSelectedBlock() {

        if (!GUIManager::levelGUIOpened()) {

            block blockToPlace = 2;

            int x = floor(selectedBlockPos_.x),
                y = floor(selectedBlockPos_.y),
                z = floor(selectedBlockPos_.z),
                hasXChanged = floor(oldSelectedBlockPos_.x) != x,
                hasYChanged = floor(oldSelectedBlockPos_.y) != y,
                hasZChanged = floor(oldSelectedBlockPos_.z) != z;

            // Avoid placing blocks not in the south, north, east, west, up or down directions of the selected block.
            if (hasXChanged + hasYChanged + hasZChanged > 1)
                x -= 1 * sign(camera_->direction().x);
            else
                if (hasXChanged)
                    x -= 1 * sign(camera_->direction().x);
                else
                    if (hasYChanged)
                        y -= 1 * sign(camera_->direction().y);
                    else
                        if (hasZChanged)
                            z -= 1 * sign(camera_->direction().z);

            // Select appropiate chunk and modify the selected block if possible.
            std::unique_lock<std::recursive_mutex> lock(chunkManager::chunksMutex());

            chunk* selectedChunk = chunkManager::selectChunkByChunkPos(x, y, z),
                * neighbor = nullptr;

            if (selectedChunk && selectedBlock_) {

                vec3 chunkRelPos(floorMod(x, SCX),
                    floorMod(y, SCY),
                    floorMod(z, SCZ));

                selectedChunk->setBlock(chunkRelPos, blockToPlace);

                chunkManager::highPriorityUpdate(selectedChunk->chunkPos());

                if (chunkRelPos.x == 0 && (neighbor = chunkManager::neighborMinusX(selectedChunk->chunkPos())))
                    chunkManager::highPriorityUpdate(neighbor->chunkPos());

                if (chunkRelPos.x == 15 && (neighbor = chunkManager::neighborPlusX(selectedChunk->chunkPos())))
                    chunkManager::highPriorityUpdate(neighbor->chunkPos());

                if (chunkRelPos.y == 0 && (neighbor = chunkManager::neighborMinusY(selectedChunk->chunkPos())))
                    chunkManager::highPriorityUpdate(neighbor->chunkPos());

                if (chunkRelPos.y == 15 && (neighbor = chunkManager::neighborPlusY(selectedChunk->chunkPos())))
                    chunkManager::highPriorityUpdate(neighbor->chunkPos());

                if (chunkRelPos.z == 0 && (neighbor = chunkManager::neighborMinusZ(selectedChunk->chunkPos())))
                    chunkManager::highPriorityUpdate(neighbor->chunkPos());

                if (chunkRelPos.z == 15 && (neighbor = chunkManager::neighborPlusZ(selectedChunk->chunkPos())))
                    chunkManager::highPriorityUpdate(neighbor->chunkPos());

            }

        }

    }

    void player::processSelectionRaycast() {

        if (game::loopSelection() == GRAPHICALLEVEL)
            chunkManager::waitTerrainLoaded();

        while (game::loopSelection() == GRAPHICALLEVEL) {

            selectBlock();

            if (destroyBlock_) {

                destroySelectedBlock();
                destroyBlock_ = false;

            }
            else
                if (placeBlock_) {

                    placeSelectedBlock();
                    placeBlock_ = false;

                }

            {

                using namespace std::chrono_literals;

                std::this_thread::sleep_for(1ms);

            }

        }

    }

    void player::changePosition(int newX, int newY, int newZ, int newDirX, int newDirY, int newDirZ) {
    
        camera_->setPos(newX, newY, newZ);
        camera_->setChunkPos(chunkManager::getChunkCoords(newX, newY, newZ));
        camera_->setDirection(newDirX, newDirY, newDirZ);
    
    }

    void player::cleanUp() {

        delete camera_;
        initialised_ = false;

    }


    // 'entity' class.

    const float entity::piDiv = 3.1415926f / 180.0f;


    entity::entity(unsigned int modelID, const vec3& pos, const vec3& rot, tickFunc func)
        : model_(&models::getModelAt(modelID)), pos_(pos), rot_(rot),
        updateXRotation_(false), updateYRotation_(false), updateZRotation_(false),
        tickFunc_(func) {
    
        if (rot != vec3Zero)
            rotate(rot);
    
    }

    void entity::rotate(const vec3& rot) {
    
        rotate(rot.x, rot.y, rot.z);
    
    }

    void entity::rotate(float x, float y, float z) {
    
        if (x) {
        
            rot_.x = x;
            updateXRotation_ = true;
        
        }

        if (y) {

            rot_.y = y;
            updateYRotation_ = true;

        }

        if (z) {

            rot_.z = z;
            updateZRotation_ = true;

        }

    }

    void entity::rotateX(float angle) {

        if (angle) {

            rot_.x = angle;
            updateXRotation_ = true;

        }

    }

    void entity::rotateY(float angle) {

        if (angle) {

            rot_.y = angle;
            updateYRotation_ = true;

        }

    }

    void entity::rotateZ(float angle) {

        if (angle) {

            rot_.z = angle;
            updateZRotation_ = true;

        }

    }


    // 'entityManager' class.

    bool entityManager::initialised_ = false;
    std::vector<entity> entityManager::entities_;
    std::vector<batch> entityManager::batches_;
    std::vector<const model*>* entityManager::renderingDataWrite_ = nullptr,
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
    unsigned int entityManager::ticksPerFrame_ = 0;

    
    void entityManager::init() {
    
        if (initialised_)
            logger::errorLog("Entity management system is already initialised");
        else {
        
            renderingDataWrite_ = new std::vector<const model*>();
            renderingDataRead_ = new std::vector<const model*>();
            ticksPerFrame_ = 30;

            initialised_ = true;
        
        }
    
    }

    bool entityManager::isEntityRegistered(unsigned int entityID) {

        std::unique_lock<std::recursive_mutex> lockEntities(entitiesMutex_);

        return entityID < entities_.size() && freeEntityID_.find(entityID) == freeEntityID_.cend();

    }

    bool entityManager::isEntityActiveAt(unsigned int entityID) {

        if (isEntityRegistered(entityID))
            return isEntityActive(entityID);
        else
            logger::errorLog("Entity with ID " + std::to_string(entityID) + " was not found");

    }

    bool entityManager::isEntityActive(unsigned int entityID) {

        std::unique_lock<std::recursive_mutex> lockEntities(entitiesMutex_);

        return inactiveEntityID_.find(entityID) == inactiveEntityID_.cend();

    }

    unsigned int entityManager::registerBatch() {

        std::unique_lock<std::recursive_mutex> lockBatches(batchesMutex_);

        batches_.emplace_back();

        return batches_.size() - 1;

    }

    void entityManager::manageEntities(std::atomic<double>& timeStep) {

        if (game::loopSelection() == GRAPHICALLEVEL)
            chunkManager::waitTerrainLoaded();

        player::changePosition(worldGen::playerSpawnPos());

        {

            std::unique_lock<std::mutex> syncLock(syncMutex_);

            unsigned int ID;
            while (game::loopSelection() == GRAPHICALLEVEL) {

                // Process active entities that have a corresponding tick function.
                entitiesMutex_.lock();

                for (unsigned int i = 0; i < ticksPerFrame_ && i < tickingEntityID_.size(); i++) { // It processes min(ticksPerFrame_, tickingEntityID_.size()) ticks.
                
                    ID = tickingEntityID_.front();
                    entities_[ID].tickFunc_();
                    tickingEntityID_.push_back(ID);
                    tickingEntityID_.pop_front();

                }

                // Delete all unused entities.
                for (auto it = deleteableEntityID_.cbegin(); it != deleteableEntityID_.cend(); it++)
                    deleteEntity(*it);
                deleteableEntityID_.clear();

                entitiesMutex_.unlock();


                // Regenerate all batches that need to be.
                batchesMutex_.lock();

                for (unsigned int i = 0; i < batches_.size(); i++)
                    if (batches_[i].isDirty())
                        renderingDataWrite_->push_back(batches_[i].generateVertices());

                batchesMutex_.unlock();


                // Sync with rendering thread and reset some structures for next iteration.
                entityManagerCV_.wait(syncLock);

                renderingDataWrite_->clear();

            }

        }

    }

    unsigned int entityManager::nEntities() {

        std::unique_lock<std::recursive_mutex> lock(entitiesMutex_);

        return entities_.size();

    }

    unsigned int entityManager::registerEntity(unsigned int modelID, int posX, int posY, int posZ, float rotX, float rotY, float rotZ, tickFunc func) {

        unsigned int entityID = 0,
                     batchID = 0;


        std::unique_lock<std::recursive_mutex> lockEntities(entitiesMutex_);

        // Get entity's ID and register it inside the 'entities_' structure.
        if (freeEntityID_.empty()) {

            entityID = entities_.size();
            entities_.emplace_back(modelID, vec3(posX, posY, posZ), vec3(rotX, rotY, rotZ), func);

        }
        else {

            entityID = *freeEntityID_.begin();
            freeEntityID_.erase(entityID);
            
            entities_[entityID].setModelID(entityID);
            entities_[entityID].pos_ = vec3(posX, posY, posZ);
            entities_[entityID].rotate(rotX, rotY, rotZ);
            entities_[entityID].tickFunc_ = func;

        }
        activeEntityID_.insert(entityID);

        if (func)
            tickingEntityID_.push_back(entityID);

        // Register the new entity inside a batch.
        std::unique_lock<std::recursive_mutex> lockBatches(batchesMutex_);
        if (batches_.empty()) { // If no batch is registered.

            batchID = registerBatch();

            if (!batches_[batches_.size() - 1].addEntity(entityID))
                logger::errorLog("Entity with ID: " + std::to_string(entityID) + " has a model too big for a batch!");

        }
        else {
        
            if (freeBatchID_.empty()) {

                if (!batches_[batches_.size() - 1].addEntity(entityID)) { // If last created batch cannot store the entity's model, then create another batch.

                    batchID = registerBatch();

                    if (!batches_[batches_.size() - 1].addEntity(entityID))
                        logger::errorLog("Entity with ID " + std::to_string(entityID) + " has a model with too many vertices for a batch.");

                }
                else
                    batchID = batches_.size() - 1;

            }
            else {

                bool found = false;
                auto it = freeBatchID_.cbegin();
                for (it; it != freeBatchID_.cend() && !found; it++) // Check if new entity's model fits into one of the already created batches.
                    found = batches_[*it].addEntity(entityID);

                if (found) {

                    if (batches_[*it].size() == BATCH_MAX_VERTEX_COUNT)
                        freeBatchID_.erase(*it);

                    batchID = *it;

                }
                else {

                    batchID = registerBatch();

                    if (!batches_[batches_.size() - 1].addEntity(entityID))
                        logger::errorLog("Entity with ID: " + std::to_string(entityID) + " has a model too big for a batch!");

                }

            }
        
        }
        
        // Associate entity and corresponding batch.
        entityBatch_[entityID] = batchID;

        return entityID;

    }

    entity& entityManager::getEntityAt(unsigned int entityID) {

        if (isEntityRegistered(entityID))
            return getEntity(entityID);
        else
            logger::errorLog("Entity with ID " + std::to_string(entityID) + " is not registered");
    
    }

    entity& entityManager::getEntity(unsigned int entityID) {

        std::unique_lock<std::recursive_mutex> lockEntities(entitiesMutex_);
        return entities_[entityID];

    }

    void entityManager::changeEntityActiveStateAt(unsigned int entityID, bool active) {

        if (isEntityRegistered(entityID)) {
        
            if (active) {

                std::unique_lock<std::recursive_mutex> lockEntities(entitiesMutex_);

                inactiveEntityID_.erase(entityID);
                activeEntityID_.insert(entityID);

                if (entities_[entityID].tickFunc_)
                    tickingEntityID_.push_back(entityID);
                

            }
            else {

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

    void entityManager::deleteEntityAt(unsigned int entityID) {

        if (isEntityRegistered(entityID))
            deleteEntity(entityID);
        else
            logger::errorLog("There is no entity with ID " + std::to_string(entityID) + "to erase");

    }

    void entityManager::deleteEntity(unsigned int entityID) {

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
        std::unique_lock<std::recursive_mutex> lockBatches(batchesMutex_);
        unsigned int batchID = entityBatch_[entityID];
        if (batches_[batchID].deleteEntity(entityID))
            deleteBatch_(batchID);

        entityBatch_.erase(entityID);

    }

    void entityManager::swapReadWrite() {

        std::vector<const model*>* aux = renderingDataRead_;

        renderingDataRead_ = renderingDataWrite_;
        renderingDataWrite_ = aux;

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

        renderingDataWrite_->clear();

        renderingDataRead_->clear();

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