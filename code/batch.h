#ifndef _VOXENG_BATCH_
#define _VOXENG_BATCH_
#include "model.h"
#include "entity.h"
#include <unordered_map>
#include <atomic>


#define BATCH_MAX_VERTEX_COUNT 10000

namespace VoxelEng {

	//////////////////////////////
	//Forward class declarations//
	//////////////////////////////
	class entity;

	///////////
	//Classes//
	///////////

	class batch {

	public:

		// Constructors.

		/*
		'batch' object constructor.
		All 'batch' objets are 'registered' in the entityManager static class by pushing their references back
		into a list belonging to said class.
		*/
		batch();


		// Observers.

		/*
		Returns the vertex data stored in the batch.
		Mutual Exclusive operation.
		*/
		const void* data();

		/*
		Return the number of vertices stored in the batch.
		Mutual Exclusive operation.
		*/
		unsigned int size();

		/*
		Get whether the batch needs to regenerate the vertices (true) or not (false).
		*/
		const std::atomic<bool>& isDirty() const;

		/*
		Get the number of entities in the batch.
		Mutual Exclusive operation.
		*/
		unsigned int nEntities();


		// Modifiers.

		/*
		The model must NOT be already translated into it's correct position.
		Returns true if the model could be added into the batch or false
		if the number of vertices of said model added to the actual vertex count
		in the batch exceeds the maximum number of vertices per batch.
		*/
		bool addEntity(entity& entity, unsigned int ID);

		/*
		Get an entity from the batch. You are able to call methods that modify said entity.
		Mutual Exclusive operation.
		*/
		entity* getEntity(unsigned int ID);

		/*
		Set whether the batch needs to regenerate the vertices (true) or not (false).
		*/
		std::atomic<bool>& isDirty();

		/*
		Deletes an entity with the identifier 'ID' inside the batch.
		If such entity does not exists, it does nothing.
		Returns true if, after deletion, the batch is empty or false otherwise.
		*/
		bool deleteEntity(unsigned int ID);

		/*
		Generates new vertices based on the entities that are in the batch and
		their positions/rotations/states...
		Automatically sets the batch as not dirty (isDirty() = false).
		WARNING. It overwrites any other vertex data stored inside the batch.
		*/
		const model* generateVertices();



	private:

		std::atomic<bool> dirty_;
		std::unordered_map<unsigned int, entity*> entities_;
		model model_;

		std::recursive_mutex mutex_;

	};

	inline const std::atomic<bool>& batch::isDirty() const {

		return dirty_;

	}

	inline std::atomic<bool>& batch::isDirty() {

		return dirty_;

	}

}

#endif