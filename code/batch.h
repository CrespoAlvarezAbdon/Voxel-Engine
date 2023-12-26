/**
* @file batch.h
* @version 1.0
* @date 20/04/2023
* @author Abdon Crespo Alvarez
* @title Batch.
* @brief Contains the definition of the batch class, which is used to group the vertex data
* of one or more entities in order to optimise the draw calls performed by the engine.
*/

#ifndef _VOXELENG_BATCH_
#define _VOXELENG_BATCH_

#include <unordered_set>
#include <atomic>
#include <cstddef>
#include <mutex>

#include "model.h"
#include "gameWindow.h"


namespace VoxelEng {

	////////////
	//Defines.//
	////////////

	/**
	* @brief The maximum number of vertices a batch can hold.
	*/
	const unsigned int BATCH_MAX_VERTEX_COUNT = 10000;


	/////////////////////////
	//Forward declarations.//
	/////////////////////////

	class entity;


	////////////
	//Classes.//
	////////////

	/**
	* @brief Collection of vertex data from the models of many entities.
	* All the models which have their vertices in the same batch will be rendered
	* in one draw call.
	*/
	class batch {

	public:

		// Constructors.

		/**
		* @brief Default constructor. It must be registered properly using the entityManager class.
		*/
		batch();

		/**
		* @brief Copy constructor. It must be registered properly using the entityManager class.
		*/
		batch(const batch& b);


		// Observers.

		/**
		* @brief Returns the vertex data stored in the batch.
		* Thread-safe operation.
		*/
		const void* data();

		/**
		* @brief Returns the number of vertices stored in the batch.
		* Thread-safe operation.
		*/
		std::size_t size();

		/**
		* @brief Get whether the batch needs to regenerate the vertices (true) or not (false).
		*/
		bool isDirty() const;

		/**
		* @brief Get the number of entities in the batch.
		* Thread-safe operation.
		*/
		unsigned int nEntities();

		/**
		* @brief Get whether the batch contains the model of the specified entity (true) or not (false).
		*/
		bool isEntityInBatchAt(entityID entityID);

		/**
		* @brief Get whether the batch contains the model of the specified entity (true) or not (false).
		*/
		bool isEntityInBatch(entityID entityID);


		// Modifiers.

		/**
		* @brief The model must NOT be already translated into it's correct position.
		* Returns true if the model could be added into the batch or false
		* if the number of vertices of said model added to the actual vertex count
		* in the batch exceeds the maximum number of vertices per batch.
		*/
		bool addEntityAt(entityID entityID);

		/**
		* @brief The model must NOT be already translated into it's correct position.
		* Returns true if the model could be added into the batch or false
		* if the number of vertices of said model added to the actual vertex count
		* in the batch exceeds the maximum number of vertices per batch.
		*/
		bool addEntity(entityID entityID);

		/**
		* @brief Returns true if the specified entity's model fits inside the batch
		* or false otherwise.
		*/
		bool doesEntityFitInside(entityID entityID);

		/**
		* @brief Set whether the batch needs to regenerate the vertices (true) or not (false).
		*/
		std::atomic<bool>& isDirty();

		/**
		* @brief Returns true if the batch no longer has any active entity after this call
		* has made the required changes or false otherwise.
		*/
		bool changeActiveStateAt(entityID entityID, bool active);

		/**
		* @brief Returns true if the batch no longer has any active entity after this call
		* has made the required changes or false otherwise.
		*/
		bool changeActiveState(entityID entityID, bool active);

		/**
		* @brief Deletes an entity with the identifier 'entityID'.
		* Returns true if, after deletion, the batch is empty or false otherwise.
		*/
		bool deleteEntityAt(entityID entityID);

		/**
		* @brief Deletes an entity with the identifier 'entityID'.
		* Returns true if, after deletion, the batch is empty or false otherwise.
		*/
		bool deleteEntity(entityID entityID);

		/**
		* @brief Generates new vertices based on the entities that are in the batch and
		* their positions/rotations/states...
		* Sets the batch as not dirty.
		* WARNING. It overwrites any other vertex data stored inside the batch.
		*/
		const model& generateVertices();


		// Clean up.

		/**
		* @brief Deletes all vertices that belong to the batch and
		* removes any previous association with any entity.
		* Sets the batch as dirty.
		*/
		void clear();

		
	private:

		std::recursive_mutex mutex_;
		std::atomic<bool> dirty_;
		std::unordered_set<unsigned int> activeEntityID_,
										 inactiveEntityID_;
		model model_;

	};

	inline bool batch::isDirty() const {

		return dirty_;

	}

	inline std::atomic<bool>& batch::isDirty() {

		return dirty_;

	}

}

#endif