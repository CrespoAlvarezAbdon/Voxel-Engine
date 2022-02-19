#include "batch.h"
#include <stdexcept>
#include <string>
#include <iterator>


namespace VoxelEng {

	batch::batch() : dirty_(false) {}

	const void* batch::data() {

		std::unique_lock<recursive_mutex> lock(mutex_);

		return model_.data();

	}

	unsigned int batch::size() {

		std::unique_lock<recursive_mutex> lock(mutex_);

		return model_.size();

	}

	bool batch::addEntity(entity& entity, unsigned int ID) {

		std::unique_lock<recursive_mutex> lock(mutex_);

		if (model_.size() + entity.entityModel().size() <= BATCH_MAX_VERTEX_COUNT)
		{

			entities_[ID] = &entity;

			return true;

		}
		else
			return false;
	
	}

	bool batch::deleteEntity(unsigned int ID) {
	
		std::unique_lock<recursive_mutex> lock(mutex_);

		if (entities_.find(ID) == entities_.end())
			throw runtime_error("No entity with ID: " + std::to_string(ID) + " exists in this batch!");
		else {

			entities_.erase(ID);

			return !entities_.size();

		}
	
	}

	const model* batch::generateVertices() {
	
		std::unique_lock<recursive_mutex> lock(mutex_);

		model_.clear();

		for (auto it = entities_.cbegin(); it != entities_.cend(); it++) {
		
			model entityModel = it->second->entityModel();

			// Translate the model's copy to the entity's position.
			for (unsigned int j = 0; j < entityModel.size(); j++) {

				entityModel[j].positions[0] += it->second->x();
				entityModel[j].positions[1] += it->second->y();
				entityModel[j].positions[2] += it->second->z();

				model_.push_back(entityModel[j]);

			}

		}

		dirty_ = false;

		return &model_;
	
	}

    entity* batch::getEntity(unsigned int ID) {

		std::unique_lock<recursive_mutex> lock(mutex_);

		if (entities_.find(ID) == entities_.end())
			throw runtime_error("No entity with ID: " + std::to_string(ID) + " exists in this batch!");
		else
			return entities_[ID];

	}

	unsigned int batch::nEntities() {

		std::unique_lock<recursive_mutex> lock(mutex_);

		return entities_.size();

	}

}