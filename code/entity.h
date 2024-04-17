/**
* @file entity.h
* @version 1.0
* @date 20/04/2023
* @author Abdon Crespo Alvarez
* @title Entity.
* @brief Contains the player, entity and entityManager classes as well as some
* auxiliary data structures and types used with them.
*/
#ifndef _VOXENG_ENTITY_
#define _VOXENG_ENTITY_

#include <atomic>
#include <concepts>
#include <condition_variable>
#include <list>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <mutex>
#include "batch.h"
#include "camera.h"
#include "chunk.h"
#include "definitions.h"
#include "model.h"
#include "vec.h"
#include "Graphics/transform.h"



namespace VoxelEng {

	/////////////////////////
	//Forward declarations.//
	/////////////////////////

	class batch;
	class entityManager;


	////////////
	//Classes.//
	////////////

	/**
	* @brief An entity possess a custom 3D model as its representation
	* inside a level and is capable of being associated with actions
	* that affect other entities or the level itself.
	*/
	class entity {

	public:

		// Constructors.

		/**
		* @brief Class constructor.
		* WARNING. To create an entity use entityManager::registerEntity(). Otherwise the
		* created entity will not be registered in the entity management system.
		*/
		entity();


		// Observers.

		/**
		* @brief Get the entity's transform.
		*/
		const transform& getTransform() const;

		/**
		* @brief Get the entity's position.
		*/
		const vec3& pos() const;

		/**
		* @brief Get entity's postion in X axis.
		*/
		float x() const;

		/**
		* @brief Get entity's postion in Y axis.
		*/
		float y() const;

		/**
		* @brief Get entity's postion in Z axis.
		*/
		float z() const;

		/**
		* @brief Get the entity's rotation.
		*/
		const vec3& rot() const;

		/**
		* @brief Get the entity's model.
		*/
		const model& entityModel() const;

		/**
		* @brief Flag to check if model X rotation needs to be updated in the model's vertices.
		*/
		bool updateXRotation() const;

		/**
		* @brief Flag to check if model Y rotation needs to be updated in the model's vertices.
		*/
		bool updateYRotation() const;

		/**
		* @brief Flag to check if model Z rotation needs to be updated in the model's vertices.
		*/
		bool updateZRotation() const;

		/**
		* @brief Get the entity's rotation mode to apply when applying the entity's rotation to its model.
		*/
		applyRotationMode getApplyRotationMode() const;
		
		/**
		* @brief Get the entity's ID.
		*/
		entityID ID() const;

		/**
		* @brief Return true if the entity has an associated tick function or false otherwise.
		*/
		bool hasTickFunction() const;


		// Modifiers.

		/**
		* @brief Set the entity's transform.
		*/
		transform& getTransform();

		/**
		* @brief Set the entity's position.
		*/
		vec3& pos();

		/**
		* @brief Set the entity's position.
		*/
		void pos(float x, float y, float z);

		/**
		* @brief Set entity's postion in X axis.
		*/
		float& x();

		/**
		* @brief Set entity's postion in Y axis.
		*/
		float& y();

		/**
		* @brief Set entity's postion in Z axis.
		*/
		float& z();

		/**
		* @brief Change the entity's model.
		*/
		void entityModel(unsigned int modelID);

		/**
		* @brief Rotate the entity.
		*/
		void rotate(const vec3& rotation);

		/**
		* @brief Rotate the entity.
		* NOTE. This is only valid with entities that have applyRotationMode::EULER_ANGLES.
		*/
		void rotate(float x, float y, float z);

		/**
		* @brief Rotate the entity's model in the X-axis
		*/
		void rotateX(float angle);

		/**
		* @brief Rotate the entity's model in the Y-axis
		*/
		void rotateY(float angle);

		/**
		* @brief Rotate the entity's model in the Z-axis
		*/
		void rotateZ(float angle);

		/**
		* @brief Rotates the entity's view direction, considered as a vector with direction
		* towards the front of the model, and consequently rotates the entity's model so that
		* said view direction vector still points to the model's front.
		* The vec3 direction is considered to contain the roll, pitch and yaw angles in said order,
		* with the roll angle being the first element.
		*/
		void rotateView(const vec3& rotation);

		/**
		* @brief Rotates the entity's view direction, considered as a vector with direction
		* towards the front of the model, and consequently rotates the entity's model so that
		* said view direction vector still points to the model's front.
		*/
		void rotateView(float roll, float pitch, float yaw);

		/**
		* @brief Rotates the entity's view direction (only the roll angle), considered as a vector with direction
		* towards the front of the model, and consequently rotates the entity's model so that
		* said view direction vector still points to the model's front.
		*/
		void rotateViewRoll(float angle);

		/**
		* @brief Rotates the entity's view direction (only the pitch angle), considered as a vector with direction
		* towards the front of the model, and consequently rotates the entity's model so that
		* said view direction vector still points to the model's front.
		*/
		void rotateViewPitch(float angle);

		/**
		* @brief Rotates the entity's view direction (only the yaw angle), considered as a vector with direction
		* towards the front of the model, and consequently rotates the entity's model so that
		* said view direction vector still points to the model's front.
		*/
		void rotateViewYaw(float angle);

		/**
		* @brief Set the entity's ID.
 		*/
		void ID(entityID ID);

		/**
		* @brief Set the entity's tick function.
		*/
		void setTickFunc(tickFunc func);

		/**
		* @brief Set the entity's apply rotation mode.
		*/
		void setApplyRotationMode(applyRotationMode rotMode);

		/**
		* @brief Execute the tick function associated to this entity.
		* Throws exception if there is no function associated.
		*/
		void executeTickFunc();

		/**
		* @brief Set the entity's up vector or Y axis.
		*/
		void setYAxis(const vec3& axis);

		/**
		* @brief Set the entity's up vector or Y axis.
		*/
		void setYAxis(float x, float y, float z);

	protected:


		/*
		Attributes.
		*/

		entityID ID_;

		bool updateXRotation_,
			 updateYRotation_,
			 updateZRotation_;
		transform transform_;
		const model* model_;

		applyRotationMode applyRotationMode_;

		tickFunc tickFunc_;

	};

	inline entity::entity()
	: ID_(0),
	model_(nullptr),
	applyRotationMode_(applyRotationMode::NOT_SPECIFIED),
	tickFunc_(nullptr) {}

	inline const transform& entity::getTransform() const {
	
		return transform_;
	
	}

	inline const vec3& entity::pos() const {

		return transform_.position;

	}

	inline const vec3& entity::rot() const {

		return transform_.rotation;

	}

	inline float entity::x() const {

		return transform_.position.x;

	}

	inline float entity::y() const {

		return transform_.position.y;

	}

	inline float entity::z() const {

		return transform_.position.z;

	}

	inline const model& entity::entityModel() const {

		return *model_;

	}

	inline bool entity::updateXRotation() const {

		return updateXRotation_;

	}

	inline bool entity::updateYRotation() const {

		return updateYRotation_;

	}

	inline bool entity::updateZRotation() const {

		return updateZRotation_;

	}

	inline applyRotationMode entity::getApplyRotationMode() const {
	
		return applyRotationMode_;
	
	}

	inline entityID entity::ID() const {
	
		return ID_;
	
	}

	inline bool entity::hasTickFunction() const {
	
		return tickFunc_;
	
	}

	inline transform& entity::getTransform() {
	
		return transform_;
	
	}

	inline vec3& entity::pos() {

		return transform_.position;

	}

	inline void entity::pos(float x, float y, float z) {

		transform_.position.x = x;
		transform_.position.y = y;
		transform_.position.z = z;

	}

	inline float& entity::x() {

		return transform_.position.x;

	}

	inline float& entity::y() {

		return transform_.position.y;

	}

	inline float& entity::z() {

		return transform_.position.z;

	}

	inline void entity::rotate(const vec3& rot) {

		rotate(rot.x, rot.y, rot.z);

	}

	inline void entity::rotateView(const vec3& rotation) {

		rotateView(rotation.x, rotation.y, rotation.z);

	}

	inline void entity::entityModel(unsigned int modelID) {

		model_ = &models::getModelAt(modelID);

	}

	inline void entity::ID(entityID ID) {
	
		ID_ = ID;
	
	}

	inline void entity::setTickFunc(tickFunc func) {

		tickFunc_ = func;

	}

	inline void entity::setApplyRotationMode(applyRotationMode rotMode) {

		applyRotationMode_ = rotMode;

	}

	inline void entity::executeTickFunc() {
	
		tickFunc_();
	
	}

	inline void entity::setYAxis(const vec3& axis) {
	
		transform_.Yaxis = axis;
	
	}

	inline void entity::setYAxis(float x, float y, float z) {

		transform_.Yaxis.x = x;
		transform_.Yaxis.y = y;
		transform_.Yaxis.z = z;

	}
		

	/**
	* @brief This class is in charge of encapsulating and abstracting
	* all related to the management of entities in the engine, including
	* the entity part of the AI agents.
	*/
	class entityManager {

	public:

		// Initialisation.

		/**
		* @brief Initialise the entity management system.
		* Allocate any resources that are needed on initialisation.
		*/
		static void init();


		// Observers.

		/**
		* @brief Returns true if the entity management system is initialised.
		*/
		static bool initialised();

		/**
		* @brief Returns true if the specified entity ID correponds to a registered one or false otherwise.
		* An entityID that belongs to a deleted (or rather "freed") entity does not count
		* as an registered entity until a new call to any overload of entityManager::registerEntity()
		* reuses said ID (and returns it as the call's result) to represent another entity.
		*/
		static bool isEntityRegistered(entityID entityID);

		/**
		* @brief Returns true if the specified entity is active (with bounds checking).
		*/
		static bool isEntityActiveAt(entityID entityID);

		/**
		* @brief Returns true if the specified entity is active (without bounds checking).
		*/
		static bool isEntityActive(entityID entityID);


		// Modifiers: general.

		/**
		* @brief Sets the entity management system's AI mode on or off.
		* Intended to be use inside the game::setAImode method only.
		*/
		static void setAImode(bool on);

		/**
		* @brief Function used by a worker thread to manage entities in a world/level.
		* Management includes creating, updating and removing entities from the world.
		*/
		static void manageEntities();

		/**
		* @brief Return the total number of entity IDs used (even for freed entity objects).
		*/
		static unsigned int nEntities();

		/**
		* @brief Spawns a new entity.
		* Returns the ID of the registered entity.
		*/
		static entityID spawnEntity(unsigned int modelID, float posX, float posY, float posZ, applyRotationMode applyRotMode = applyRotationMode::EULER_ANGLES, 
			float rotX = 0, float rotY = 0, float rotZ = 0, tickFunc func = nullptr);

		/**
		* @brief Spawns a new entity of type T, with T being a type that derives from entity.
		* Returns the ID of the registered entity.
		*/
		template<typename T = entity>
		requires std::derived_from<T, entity>
		static entityID spawnEntity(float posX, float posY, float posZ, float rotX = 0, float rotY = 0, float rotZ = 0);

		/**
		* @brief Returns the entity with the specified entity ID (with bounds checking).
		*/
		static entity& getEntityAt(entityID ID);

		/**
		* @brief Returns the entity with the specified entity ID (without bounds checking).
		*/
		static entity& getEntity(entityID ID);

		/**
		* @brief Changes the entity's active state (without bounds checking).
		*/
		static void changeEntityActiveStateAt(entityID ID, bool active);

		/**
		* @brief Deletes an existing entity from the entity management system.
		* The entity ID remains free to be reused for other new entities.
		*/
		static void deleteEntityAt(entityID ID);

		/**
		* @brief Deletes an existing entity from the entity management system.
		* The entity ID remains free to be reused for other new entities.
		*/
		static void deleteEntity(entityID ID);

		/**
		* @brief Swaps read-only and write-only batch rendering data.
		* WARNING. Rendering-thread and entity management thread must be synced when calling this function
		*/
		static void swapReadWrite();

		/**
		* @brief Get condition variable used for syncing the rendering and the batching threads.
		*/
		static std::condition_variable& entityManagerCV();

		/**
		* @brief Get condition variable flag used for syncing the rendering and the batching threads.
		*/
		static bool syncFlag();

		/**
		* @brief Get mutex used for syncing the rendering and the batching threads.
		*/
		static std::mutex& syncMutex();

		/**
		* @brief Get the readable rendering data necessary for the rendering thread to render the batches properly.
		*/
		static const std::vector<model>* renderingData();


		// Modifiers.

		/**
		* @brief Move the entity.
		*/
		static void moveEntity(entityID entityID, float x, float y, float z);

		/**
		* @brief Set the entity's transform, marking its mesh as dirty in the process.
		*/
		static void setTransform(entityID ID, transform newTransform);

		
		// Clean up.

		/**
		* @brief Free dynamic memory allocated by the entity manager system and NOT deinitialise it.
		*/
		static void clear();

		/**
		* @brief Free dynamic memory allocated by the entity manager system and deinitialise it.
		*/
		static void reset();

	private:

		/*
		Attributes.
		*/

		static bool initialised_,
					firstManagementIteration_;
		static std::vector<entity*> entities_;
		static std::vector<batch> batches_;

		static std::unordered_map<unsigned int, unsigned int> entityBatch_; // Relates entity's ID with the batch it belongs to.
		static std::unordered_set<unsigned int> activeEntityID_,
												activeBatchID_,
											    freeEntityID_,
												freeBatchID_,
												inactiveEntityID_,
												inactiveBatchID_,
												deleteableEntityID_;

		// Queue of entities' ID that belong to entities that are
		// waiting for their tick method (with its respective pointer different from nullptr)
		// to execute.
		static std::list<unsigned int> tickingEntityID_; 	

		static std::vector<model>* renderingDataWrite_,
						         * renderingDataRead_;
		
		static std::recursive_mutex entitiesMutex_,
									batchesMutex_;
		static std::condition_variable entityManagerCV_;
		static std::atomic<bool> entityMngCVContinue_;
		static std::mutex syncMutex_;
		static std::unique_lock<std::mutex> syncLock_;
		static unsigned int ticksPerFrame_, // Used to distribute tick function executions between frame.
							opsPerFrame_; // Used to distribute some operations between frames.


		/*
		Methods.
		*/

		// Observers.

		/*
		Note: freed batches do not count as registered as they
		are 'deleted' but they actually are not in order to avoid
		unnecessary allocations.
		*/
		static bool isBatchRegistered_(unsigned int batchID);

		static bool isBatchActiveAt_(unsigned int batchID);

		static bool isBatchActive_(unsigned int batchID);


		// Modifiers.

		/*
		Method called by a batch automatically while it is being constructed to register itself
		in the entityManager's batch list. This allows the manager to access to all the entities in the world.
		Batches are used to save drawing calls, but the game logic for the entities is calculated by the method
		entityManager::manager.
		Returns the ID of the registered batch.
		*/
		static unsigned int registerBatch_();

		static void changeBatchActiveStateAt_(unsigned int batchID, bool active);

		static void changeBatchActiveState_(unsigned int batchID, bool active);

		/*
		Marks a batch as free to be reused later.
		*/
		static void deleteBatchAt_(unsigned int batchID);

		/*
		Marks a batch as free to be reused later.
		*/
		static void deleteBatch_(unsigned int batchID);

		/*
		Insert entity into the entity manager system.
		NOTE. Entity must be created with a entityManager::spawnEntity method.
		*/
		static entityID insertEntity_(entity* entityToInsert);

	};

	inline bool entityManager::initialised() {
	
		return initialised_;
	
	}

	template<typename T>
	requires std::derived_from<T, entity>
	entityID entityManager::spawnEntity(float posX, float posY, float posZ, float rotX, float rotY, float rotZ) {

		entity* createdEntity = new T();
		
		createdEntity->pos(posX, posY, posZ);
		applyRotationMode applyRotMode = createdEntity->getApplyRotationMode();
		if (applyRotMode == applyRotationMode::EULER_ANGLES)
			createdEntity->rotate(rotX, rotY, rotZ);
		else if (applyRotMode == applyRotationMode::DIRECTION_VECTOR)
			createdEntity->setYAxis(rotX, rotY, rotZ);

		return insertEntity_(createdEntity);

	}

	inline std::condition_variable& entityManager::entityManagerCV() {
	
		return entityManagerCV_;
	
	}

	inline bool entityManager::syncFlag() {
	
		return entityMngCVContinue_;
		
	}

	inline std::mutex& entityManager::syncMutex() {
	
		return syncMutex_;
	
	}

	inline const std::vector<model>* entityManager::renderingData() {
	
		return renderingDataRead_;
	
	}

}

#endif