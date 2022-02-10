#include "batch.h"
#include <stdexcept>

namespace VoxelEng {

	batch::batch() {
	
		entityManager.
	
	}

	bool batch::addEntity(entity& entity) {
	
		unsigned int modelSize = entity.entityModel().size();

		if (vertexCount_ + modelSize <= BATCH_MAX_VERTEX_COUNT)
		{

			if (freeInd_.empty())
			{

				entity.batchID() = entities_.size();
				entities_.push_back(&entity);

			}
			else
			{

				unsigned int ID = freeInd_.front();

				entities_[ID] = &entity;
				entity.batchID() = ID;
				freeInd_.pop_front();

			}

			vertexCount_ += modelSize;
			return true;

		}
		else
			return false;
	
	}

	void batch::deleteEntity(unsigned int ID) {
	
		if (ID == entities_.size() - 1)
			entities_.pop_back();
		else
			if (ID >= entities_.size())
				throw runtime_error("The specified ID does not exist inside the batch!");
			else {
			
				entities_[ID] = nullptr;
				freeInd_.push_back(ID);
			
			}
	
	}

	void batch::generateVertices() {
	
		model_.clear();

		for (int i = 0; i < entities_.size(); i++) {
		
			model entityModel = entities_[i]->entityModel();

			// Translate the model's copy to the entity's position.
			for (unsigned int j = 0; j < entityModel.size(); j++) {

				entityModel[j].positions[0] += entities_[i]->x();
				entityModel[j].positions[1] += entities_[i]->y();
				entityModel[j].positions[2] += entities_[i]->z();

				model_.push_back(entityModel[j]);

			}

		}
	
	}

}