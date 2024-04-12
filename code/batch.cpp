#include "batch.h"

#include <string>
#include <iterator>

#include "entity.h"
#include "definitions.h"
#include "logger.h"


namespace VoxelEng {


	// 'batch' class.

	batch::batch() 
	: dirty_(false) {}

	batch::batch(const batch& b) 
	: dirty_(b.dirty_.load()), activeEntityID_(b.activeEntityID_), inactiveEntityID_(b.inactiveEntityID_), model_(b.model_) {}

	const void* batch::data() {

		std::unique_lock<std::recursive_mutex> lock(mutex_);

		return model_.data();

	}

	std::size_t batch::size() {

		std::unique_lock<std::recursive_mutex> lock(mutex_);

		return model_.size();

	}

	unsigned int batch::nEntities() {

		std::unique_lock<std::recursive_mutex> lock(mutex_);

		return activeEntityID_.size();

	}

	bool batch::isEntityInBatchAt(entityID entityID) {
	
		std::unique_lock<std::recursive_mutex> lock(mutex_);

		if (entityManager::isEntityRegistered(entityID))
			return activeEntityID_.find(entityID) != activeEntityID_.cend() ||
				   inactiveEntityID_.find(entityID) != inactiveEntityID_.cend();
		else
			logger::errorLog("No entity with ID " + std::to_string(entityID) + " is registered");
	
	}

	bool batch::isEntityInBatch(entityID entityID) {

		std::unique_lock<std::recursive_mutex> lock(mutex_);

		return activeEntityID_.find(entityID) != activeEntityID_.cend() ||
			   inactiveEntityID_.find(entityID) != inactiveEntityID_.cend();

	}

	bool batch::addEntityAt(entityID entityID) {

		if (entityManager::isEntityRegistered(entityID)) {
		
			if (isEntityInBatch(entityID))
				logger::errorLog("Entity with ID: " + std::to_string(entityID) + " is already registered in this batch");
			else
				return addEntity(entityID);
		
		}
		else
			logger::errorLog("No entity with ID " + std::to_string(entityID) + " is registered");

	}

	bool batch::addEntity(entityID entityID) {

		std::unique_lock<std::recursive_mutex> lock(mutex_);

		if (model_.size() + entityManager::getEntity(entityID).entityModel().size() <= BATCH_MAX_VERTEX_COUNT) {

			activeEntityID_.insert(entityID);
			dirty_ = true;

			return true;

		}
		else
			return false;

	}

	bool batch::doesEntityFitInside(entityID entityID) {

		std::unique_lock<std::recursive_mutex> lock(mutex_);

		if (model_.size() + entityManager::getEntity(entityID).entityModel().size() <= BATCH_MAX_VERTEX_COUNT)
			return true;
		else
			return false;

	}

	bool batch::changeActiveStateAt(entityID entityID, bool active) {

		if (entityManager::isEntityRegistered(entityID))
			if (isEntityInBatch(entityID))
				return changeActiveState(entityID, active);
			else
				logger::errorLog("Entity with ID " + std::to_string(entityID) + " is not in this batch");
		else
			logger::errorLog("Entity with ID " + std::to_string(entityID) + " was not found");

	}

	bool batch::changeActiveState(entityID entityID, bool active) {

		if (active) {

			inactiveEntityID_.erase(entityID);
			activeEntityID_.insert(entityID);

		}
		else {

			inactiveEntityID_.insert(entityID);
			activeEntityID_.erase(entityID);

		}

		return !activeEntityID_.size();
	}

	bool batch::deleteEntityAt(entityID entityID) {

		if (entityManager::isEntityRegistered(entityID))
			logger::errorLog("No entity with ID: " + std::to_string(entityID) + " is registered");
		else {

			if (isEntityInBatch(entityID))
				return deleteEntity(entityID);
			else
				logger::errorLog("No entity with ID: " + std::to_string(entityID) + " exists in this batch");;

		}

	}

	bool batch::deleteEntity(entityID entityID) {

		std::unique_lock<std::recursive_mutex> lock(mutex_);

		if (entityManager::isEntityActive(entityID))
			activeEntityID_.erase(entityID);
		else
			inactiveEntityID_.erase(entityID);

		dirty_ = true;

		return !(activeEntityID_.size() + inactiveEntityID_.size());

	}
	
	const model& batch::generateVertices() {

		float sinAngleX,
			  cosAngleX,
			  sinAngleY,
			  cosAngleY,
			  sinAngleZ,
			  cosAngleZ,
			  oldFirstCoord,
			  oldSecondCoord;
		model entityModel;
		bool updateXRot,
			 updateYRot,
			 updateZRot;


		std::unique_lock<std::recursive_mutex> lock(mutex_);
		model_ = model(); // TODO. HACER QUE SE "REUSEN" VERTICES Y SI HAY UN MENOR NUMERO SE ELIMINEN SOLO LOS SOBRANTES PARA AHORRAR TIEMPO DE EJECUCIÓN?
		for (auto it = activeEntityID_.cbegin(); it != activeEntityID_.cend(); it++) {

			entity& selectedEntity = entityManager::getEntity(*it);
			entityModel = selectedEntity.entityModel();

			models::applyTransform(entityModel, selectedEntity.getTransform(), model_, applyRotationMode::EULER_ANGLES,
				selectedEntity.updateXRotation(), selectedEntity.updateYRotation(), selectedEntity.updateZRotation());

		}

		dirty_ = false;

		return model_;

	}

	void batch::clear() {
	
		std::unique_lock<std::recursive_mutex> lock(mutex_);

		dirty_ = true;

		for (std::size_t i = 0; i < activeEntityID_.size(); i++)
			entityManager::deleteEntity(i);
		activeEntityID_.clear();

		for (std::size_t i = 0; i < inactiveEntityID_.size(); i++)
			entityManager::deleteEntity(i);
		inactiveEntityID_.clear();

		model_.clear();
	
	}

}