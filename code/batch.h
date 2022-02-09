#ifndef _VOXENG_BATCH_
#define _VOXENG_BATCH_
#include "model.h"
#include "entity.h"
#include <vector>
#include <deque>


#define BATCH_MAX_VERTEX_COUNT 10000

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
		bool addEntity(entity& entity);


		/*
		Deletes an entity with the identifier 'ID' inside the batch.
		If such entity does not exists, it does nothing.
		*/
		void deleteEntity(unsigned int ID);

		/*
		Generates new vertices based on the entities that are in the batch and
		their positions/rotations/states...
		WARNING. It overwrites any other vertex data stored inside the batch.
		*/
		void generateVertices();



	private:

		std::vector<entity*> entities_;
		std::deque<unsigned int> freeInd_;
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