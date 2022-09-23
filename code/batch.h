#ifndef _VOXELENG_BATCH_
#define _VOXELENG_BATCH_
#include "model.h"
#include "entity.h"
#include "gameWindow.h"
#include <unordered_set>
#include <atomic>
#include <cstddef>




namespace VoxelEng {

	////////////
	//Defines.//
	////////////

	#define BATCH_MAX_VERTEX_COUNT 10000


	/////////////////////////
	//Forward declarations.//
	/////////////////////////

	class entity;


	////////////
	//Classes.//
	////////////

	class batch {

	public:

		// Constructors.

		/*
		'batch' object constructor.
		All 'batch' objets are 'registered' in the entityManager static class.
		*/
		batch();

		batch(const batch& b);


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
		std::size_t size();

		/*
		Get whether the batch needs to regenerate the vertices (true) or not (false).
		*/
		const std::atomic<bool>& isDirty() const;

		/*
		Get the number of entities in the batch.
		Mutual Exclusive operation.
		*/
		unsigned int nEntities();

		bool isEntityInBatchAt(unsigned int entityID);

		bool isEntityInBatch(unsigned int entityID);

		// Modifiers.

		/*
		The model must NOT be already translated into it's correct position.
		Returns true if the model could be added into the batch or false
		if the number of vertices of said model added to the actual vertex count
		in the batch exceeds the maximum number of vertices per batch.
		*/
		bool addEntityAt(unsigned int entityID);

		/*
		The model must NOT be already translated into it's correct position.
		Returns true if the model could be added into the batch or false
		if the number of vertices of said model added to the actual vertex count
		in the batch exceeds the maximum number of vertices per batch.
		*/
		bool addEntity(unsigned int entityID);

		/*
		Set whether the batch needs to regenerate the vertices (true) or not (false).
		*/
		std::atomic<bool>& isDirty();

		/*
		Returns true if the batch no longer has any active entity after this call
		has made the required changes or false otherwise.
		*/
		bool changeActiveStateAt(unsigned int entityID, bool active);

		/*
		Returns true if the batch no longer has any active entity after this call
		has made the required changes or false otherwise.
		*/
		bool changeActiveState(unsigned int entityID, bool active);

		/*
		Deletes an entity with the identifier 'entityID'.
		Returns true if, after deletion, the batch is empty or false otherwise.
		*/
		bool deleteEntityAt(unsigned int entityID);

		/*
		Deletes an entity with the identifier 'entityID'.
		Returns true if, after deletion, the batch is empty or false otherwise.
		*/
		bool deleteEntity(unsigned int entityID);

		/*
		Generates new vertices based on the entities that are in the batch and
		their positions/rotations/states...
		Sets the batch as not dirty.
		WARNING. It overwrites any other vertex data stored inside the batch.
		*/
		const model* generateVertices();

		/*
		Deletes all vertices that belong to the batch and
		removes any previous association with any entity.
		Sets the batch as dirty.
		*/
		void clear();


	private:

		std::atomic<bool> dirty_;
		std::unordered_set<unsigned int> activeEntityID_,
										 inactiveEntityID_;
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