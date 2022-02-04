#ifndef _VOXENG_BATCH_
#define _VOXENG_BATCH_
#include "model.h"
#include "entity.h"

#define BATCH_MAX_VERTEX_COUNT 50000

namespace VoxelEng {

	class batch {

	public:

		// Observers.

		/*
		Returns the vertex data stored in the batch.
		*/
		const void* data();

		/*
		Return the number of vertices stored in the batch.
		*/
		unsigned int size();


		// Modifiers.

		/*
		The model must NOT be already translated into it's correct position.
		Returns true if the model could be added into the batch or false
		if the number of vertices of said model added to the actual vertex count
		in the batch exceeds the maximum number of vertices per batch.
		*/
		bool appendModel(const entity& entity);


		// TODO. ADD A UPDATE MODEL METHOD. REMEMBER: model IS A STD::VECTOR SO YOU CAN KEEP TRACK OF THE MODELS' VERTICES. USE A LIST OF STARTING INDEX FOR EACH APPENDED MODEL.



	private:

		model model_;

	};

	inline const void* batch::data() {
	
		return model_.data();
	
	}

	inline unsigned int batch::size() {
	
		return model_.size();
	
	}

}

#endif