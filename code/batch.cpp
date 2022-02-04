#include "batch.h"

namespace VoxelEng {

	bool batch::appendModel(const entity& entity) {

		model model = entity.entityModel();

		if (model.size() + model_.size() <= BATCH_MAX_VERTEX_COUNT) {

			model_.insert(model_.end(), model.begin(), model.end());
			return true;

		}
		else
			return false;
	
	}

}