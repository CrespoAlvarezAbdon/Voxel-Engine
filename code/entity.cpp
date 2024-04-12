#include "entity.h"
#include <cmath>
#include <cstddef>
#include <string>
#include "camera.h"
#include "game.h"
#include "graphics/graphics.h"
#include "logger.h"
#include "utilities.h"
#include "worldGen.h"

namespace VoxelEng {

    // 'entity' class.

    entity::entity(entityID ID, unsigned int modelID, const vec3& pos, const vec3& rot, tickFunc func)
    : ID_(ID),
    model_(&models::getModelAt(modelID)),
    updateXRotation_(true), updateYRotation_(true), updateZRotation_(true),
    tickFunc_(func) {
    
        transform_.position = pos;
        rotate(rot);
    
    }

    void entity::rotate(float x, float y, float z) {
    
        if (x != 0) {
        
            transform_.rotation.x += x;
            updateXRotation_ = true;
        
        }

        if (y != 0) {

            transform_.rotation.y += y;
            updateYRotation_ = true;

        }

        if (z != 0) {

            transform_.rotation.z += z;
            updateZRotation_ = true;

        }

    }

    void entity::rotateX(float angle) {

        if (angle) {

            transform_.rotation.x += angle;
            updateXRotation_ = true;

        }

    }

    void entity::rotateY(float angle) {

        if (angle) {

            transform_.rotation.y += angle;
            updateYRotation_ = true;

        }

    }

    void entity::rotateZ(float angle) {

        if (angle) {

            transform_.rotation.z += angle;
            updateZRotation_ = true;

        }

    }

    void entity::rotateView(float roll, float pitch, float yaw) {

        if (roll != 0) {

            transform_.rotation.z += roll;
            updateZRotation_ = true;

        }

        if (pitch != 0) {

            transform_.rotation.x += pitch;
            updateXRotation_ = true;

        }

        if (yaw != 0) {

            transform_.rotation.y += yaw;
            updateYRotation_ = true;

        }

    }

    void entity::rotateViewRoll(float angle) {
    
        if (angle != 0) {

            transform_.rotation.z += angle;
            updateZRotation_ = true;

        }
    
    }

    void entity::rotateViewPitch(float angle) {
    
        if (angle != 0) {

            transform_.rotation.x += angle;
            updateXRotation_ = true;

        }
    
    }

    void entity::rotateViewYaw(float angle) {
    
        if (angle != 0) {

            transform_.rotation.y += angle;
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

        // Process active entities that have a corresponding tick function.
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
            entities_.emplace_back(entityID, modelID, vec3{ posX, posY, posZ }, vec3{ rotX, rotY, rotZ }, func);

        }
        else {

            entityID = *freeEntityID_.begin();
            freeEntityID_.erase(entityID);
            
            entity& repurposedEntity = entities_[entityID];
            repurposedEntity.ID(entityID);
            repurposedEntity.setModelID(modelID);
            repurposedEntity.transform_.position = vec3{ posX, posY, posZ };
            repurposedEntity.rotate(rotX, rotY, rotZ);
            repurposedEntity.tickFunc_ = func;

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
        entities_[entityID].transform_.rotation = vec3Zero;
        
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

            if (!game::AImodeON())
                batches_[entityBatch_[entityID]].isDirty() = true;

        }
        else
            logger::errorLog("Entity with ID " + std::to_string(entityID) + " was not found");
    
    }

    void entityManager::setTransform(entityID ID, transform newTransform) {

        if (entityManager::isEntityRegistered(ID)) {

            entity& theEntity = entityManager::getEntity(ID);
            theEntity.getTransform() = newTransform;

            if (!game::AImodeON())
                batches_[entityBatch_[ID]].isDirty() = true;

        }
        else
            logger::errorLog("Entity with ID " + std::to_string(ID) + " was not found");

    }

    void entityManager::clear() {

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

    void entityManager::reset() {

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