#include "entity.h"
#include "graphics.h"
#include "utilities.h"
#include "definitions.h"
#include <stdexcept>
#include <string>
#include <cmath>


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
    const glm::vec3& dir = camera_.direction(),
                     pos = camera_.pos();
    selectedBlock_ = 0;


    while (step < blockReachRange_ && !selectedBlock_)
    {

        selectedBlockPos_ = pos + (dir * step);

        selectedBlock_ = chunkMng_->getBlock(floor(selectedBlockPos_.x), floor(selectedBlockPos_.y), floor(selectedBlockPos_.z));

        if (!selectedBlock_) // Continue searching
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

    int x = floor(selectedBlockPos_.x),
        y = floor(selectedBlockPos_.y),
        z = floor(selectedBlockPos_.z),
        hasXChanged = floor(oldSelectedBlockPos_.x) != x,
        hasYChanged = floor(oldSelectedBlockPos_.y) != y,
        hasZChanged = floor(oldSelectedBlockPos_.z) != z;

    // Avoid placing blocks not in the south, north, east, west, up or down directions of the selected block.
    if (hasXChanged + hasYChanged + hasZChanged > 1)
        x -= 1 * VoxelEng::sign(camera_.direction().x);
    else
        if (hasXChanged)
            x -= 1 * VoxelEng::sign(camera_.direction().x);
        else
            if (hasYChanged)
                y -= 1 * VoxelEng::sign(camera_.direction().y);
            else
                if (hasZChanged)
                    z -= 1 * VoxelEng::sign(camera_.direction().z);

    // Select appropiate chunk and modify the selected block if possible.
    unique_lock<recursive_mutex> lock(chunkMng_->chunksMutex());

    chunk* selectedChunk = chunkMng_->selectChunkByChunkPos(x, y, z);

    if (selectedChunk && selectedBlock_)
    {

        chunkRelativePos chunkRelPos(VoxelEng::floorMod(x, SCX),
                                     VoxelEng::floorMod(y, SCY),
                                     VoxelEng::floorMod(z, SCZ));


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

    // 'entity' class

    entity::entity(unsigned int modelID, float x, float y, float z)
        : model_(models::getModel(modelID)), x_(x), y_(y), z_(z), 
          axis_(0), cosAngle_(0.0f), sinAngle_(0.0f){}

    void entity::rotateX(float angle) {

        sinAngle_ = std::sin(angle * 3.1415926f / 180.0f);
        cosAngle_ = std::cos(angle * 3.1415926f / 180.0f);
        axis_ = X_AXIS;

    }

    void entity::rotateY(float angle) {

        sinAngle_ = std::sin(angle * 3.1415926f / 180.0f);
        cosAngle_ = std::cos(angle * 3.1415926f / 180.0f);
        axis_ = Y_AXIS;

    }

    void entity::rotateZ(float angle) {
    
        sinAngle_ = std::sin(angle * 3.1415926f / 180.0f);
        cosAngle_ = std::cos(angle * 3.1415926f / 180.0f);
        axis_ = Z_AXIS;
    
    }


    // 'entityManager' class

    std::vector<batch*> entityManager::batches_;
    std::vector<const model*>* entityManager::renderingDataWrite_ = new std::vector<const model*>(),
                             * entityManager::renderingDataRead_ = new std::vector<const model*>();
    std::deque<unsigned int> entityManager::freeBatchInd_,
                             entityManager::freeEntityID_;
    std::unordered_map<unsigned int, entity*> entityManager::entityIDList_;
    std::unordered_map<unsigned int, unsigned int> entityManager::entityBatch_;
    std::recursive_mutex entityManager::entityIDListMutex_,
                         entityManager::batchesMutex_;
    std::condition_variable entityManager::entityManagerCV_;
    std::atomic<bool> entityManager::entityMngCVContinue_ = false;
    std::mutex entityManager::syncMutex_;


    unsigned int entityManager::registerBatch(batch* batch) {

        std::unique_lock<std::recursive_mutex> lockBatches(batchesMutex_);

        batches_.push_back(batch);

        return batches_.size() - 1;
    
    }

    void entityManager::manageEntities(atomic<double>& timeStep, const atomic<bool>& appFinished) {
    
        bool once = true,
             deletedEntity = false;
        float rotation = 0.0f;

        {

            std::unique_lock<std::mutex> syncLock(syncMutex_);

            while (!appFinished)
            {

                // Spawn entities (W.I.P).
                if (once)
                {

                    registerEntity(*(new VoxelEng::entity(14, 0, 145, 0)));
                    registerEntity(*(new VoxelEng::entity(14, 0, 148.5, 0)));
                    registerEntity(*(new VoxelEng::entity(14, 3, 150, 0)));
                    registerEntity(*(new VoxelEng::entity(14, -3.5, 149.2, 0)));

                    once = false;

                }


                // Update all existing entities.
                batches_[0]->getEntity(0)->z() -= 0.1f * timeStep;
                batches_[0]->getEntity(1)->z() -= 0.05f * timeStep;
                batches_[0]->getEntity(1)->rotateZ(rotation);
                batches_[0]->getEntity(2)->rotateX(rotation);
                batches_[0]->getEntity(3)->rotateY(rotation);
                rotation += 5 * timeStep;
                batches_[0]->isDirty() = true;

                if (!deletedEntity)
                {
                    // Remove any unnecesary entities.
                    if (batches_[0]->getEntity(0)->z() <= -1)
                    {

                        removeEntity(0);
                        registerEntity(*(new VoxelEng::entity(14, 0, 150, 0)));

                        deletedEntity = true;

                    }

                }


                // Regenerate all batches.
                for (unsigned int i = 0; i < batches_.size(); i++)
                    if (batches_[i]->isDirty())
                        renderingDataWrite_->push_back(batches_[i]->generateVertices());


                // Sync with rendering thread and reset some structures for next iteration.
                entityManagerCV_.wait(syncLock);

                renderingDataWrite_->clear();

            }

        }

    }

    unsigned int entityManager::nEntities() {
    
        std::unique_lock<recursive_mutex> lock(entityIDListMutex_);

        return entityIDList_.size();
    
    }

    void entityManager::registerEntity(entity& entity) {
    
        unsigned int entityID,
                     batchID;


        std::unique_lock<recursive_mutex> lockBatches(batchesMutex_);
        std::unique_lock<recursive_mutex> lockEntities(entityIDListMutex_);

        if (freeEntityID_.empty())
            entityID = entityIDList_.size();
        else {
        
            entityID = freeEntityID_.front();
            freeEntityID_.pop_front();
        
        }


        // Register the entity inside a batch.
        if (!batches_.empty())
        {

            if (freeBatchInd_.empty())
            {

                if (!batches_[batches_.size() - 1]->addEntity(entity, entityID)) { // If last created batch cannot store the entity's model, then create another batch.

                    registerBatch(new batch());

                    if (!batches_[batches_.size() - 1]->addEntity(entity, entityID))
                        throw runtime_error("Entity with ID: " + std::to_string(entityID) + " has a model too big for a batch!");

                }

                batchID = batches_.size() - 1;

            }
            else {
            
                bool found = false;
                unsigned int aux,
                             i;
                for (i = 0; i < freeBatchInd_.size() && !found; i++) {
                
                    found = batches_[freeBatchInd_[i]]->addEntity(entity, entityID);

                    if (!found) { // Put freeBatchind_.front() in the back and try with the next one.
                    
                        aux = freeBatchInd_.front();

                        freeBatchInd_.pop_front();

                        freeBatchInd_.push_back(aux);
                    
                    }
                
                }

                if (!found) {

                    registerBatch(new batch());

                    if (!batches_[batches_.size() - 1]->addEntity(entity, entityID))
                        throw runtime_error("Entity with ID: " + std::to_string(entityID) + " has a model too big for a batch!");

                    batchID = batches_.size() - 1;

                }
                else
                    batchID = freeBatchInd_[i];

            }

        }
        else { // If no batch is actually registered.

            registerBatch(new batch());

            if (!batches_[batches_.size() - 1]->addEntity(entity, entityID))
                throw runtime_error("Entity with ID: " + std::to_string(entityID) + " has a model too big for a batch!");

            batchID = batches_.size() - 1;

        }


        // Register entity ID in entityManager.
        if (entityIDList_.find(entityID) == entityIDList_.end()) { // If another entity with the same ID does not exist.

            entityIDList_[entityID] = &entity;
            entityBatch_[entityID] = batchID;

        }
        else
            throw runtime_error("Two entities with ID: " + std::to_string(entityID) + " is not possible!");
  
    }

    void entityManager::removeEntity(unsigned int ID) {
    
        std::unique_lock<recursive_mutex> lockBatches(batchesMutex_);
        std::unique_lock<recursive_mutex> lockEntities(entityIDListMutex_);

        if (entityBatch_.find(ID) == entityBatch_.end())
            throw runtime_error("There is no entity with ID " + std::to_string(ID));
        else {

            // Free the removed entity's ID.
            if (ID != entityIDList_.size() - 1)
                freeEntityID_.push_back(ID);

            // Remove entity from its corresponding batch and
            // mark batch as free if no more entities are related to it.
            if (batches_[entityBatch_[ID]]->deleteEntity(ID))
                freeBatchInd_.push_back(entityBatch_[ID]);

            // Update maps.
            entityIDList_.erase(ID);
            entityBatch_.erase(ID);

        }

    }

    void entityManager::swapReadWrite() {
    
        std::vector<const model*>* aux = renderingDataRead_;

        renderingDataRead_ = renderingDataWrite_;
        renderingDataWrite_ = aux;

    }

}