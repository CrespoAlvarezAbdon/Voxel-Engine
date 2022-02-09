#include "batch.h"

namespace VoxelEng {

	bool batch::addEntity(entity& entity) {
	
		if (freeInd_.empty())
		{

			entity.batchID() = entities_.size();
			entities_.push_back(&entity);
			
		}
		else
		{

			entities_[freeInd_.front()] = &entity;
			entity.batchID() = freeInd_.front();
			freeInd_.pop_front();

		}
	
	}

	void batch::deleteEntity(unsigned int ID) {
	
		if (ID == entities_.size() - 1)
			entities_.pop_back();
		else
		{

			entities_[ID] = nullptr;
			freeInd_.push_back(ID);

		}
	
	}

	void batch::generateVertices() {
	
		
	
	}

}