#ifndef _VOXENG_ENTITY_
#define _VOXENG_ENTITY_
#include <vector>
#include <mutex>
#include <atomic>
#include <condition_variable>
#include <unordered_map>
#include <unordered_set>
#include <list>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm.hpp>
#include "camera.h"
#include "chunk.h"
#include "gameWindow.h"
#include "batch.h"
#include "model.h"
#include "gameWindow.h"
#include "definitions.h"


namespace VoxelEng {

	/////////////////////////
	//Forward declarations.//
	/////////////////////////

	class batch;
	class entityManager;


	////////////
	//Classes.//
	////////////

	/*
	Abstraction that contains everything that defines the player (NOT the player model/entity in the level).
	*/
	class player {

	public:

		// Initialisation.

		/*
		Spawns the player in the world and initialices its corresponding camera.
		*/
		static void init(float FOV, float zNear, float zFar, window& window, unsigned int blockReachRange);


		// Modifiers.

		static camera& getCamera();

		/*
		Fakes a raycast to select a block in the world that is
		reachable from where the player stands.
		*/
		static void selectBlock();

		/*
		Callback function to get the player's mouse buttons
		input.
		*/
		static void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods);

		/*
		Destroys the block selected by the player's camera, if any.
		*/
		static void destroySelectedBlock();

		/*
		Places the current selected block in the hotbar (W.I.P) where the player is looking at.
		Cannot place a block on thin air, must be looking into a solid block to place.
		*/
		static void placeSelectedBlock();

		/*
		Function to process player's raycast used to interact with the level's enviroment.
		WARNING. Must not be called in the rendering thread.
		*/
		static void processSelectionRaycast();

		/*
		In the future, when an inventory system is added, this will be depreceated and deleted
		with previous warning.
		*/
		static void setBlockToPlace(block blockID);

		static void changePosition(const vec3& newPos, const vec3& newDir = vec3Zero);

		static void changePosition(int newX, int newY, int newZ, int newDirX = 0, int newDirY = 0, int newDirZ = 0);


		// Clean up.

		/*
		Used to clean up the heap memory allocated by this system.
		*/
		static void cleanUp();

	private:

		static bool initialised_;
		static GLFWwindow* window_;
		static camera* camera_;
		static float blockReachRange_,
			         blockSearchIncrement_;
		static block selectedBlock_,
			         blockToPlace_;
		static vec3 selectedBlockPos_,
			        oldSelectedBlockPos_;
		

		/*
		Flags used to coordinate the callbacks called
		on the rendering thread with the input processing thread.
		*/
		static std::atomic<bool> destroyBlock_,
					      placeBlock_;

	};

	inline camera& player::getCamera() {

		return *camera_;

	}

	inline void player::setBlockToPlace(block blockID) {
	
		blockToPlace_ = blockID;
	
	}

	inline void player::changePosition(const vec3& newPos, const vec3& newDir) {
	
		changePosition(newPos.x, newPos.y, newPos.z, newDir.x, newDir.y, newDir.z);
	
	}


	class entity {

	public:

		// Constructors.

		/*
		WARNING. To create an entity use entityManager::registerEntity(). Otherwise the
		created entity will not be registered in the entity management system.
		*/
		entity(unsigned int modelID, const vec3& pos, const vec3& rot, tickFunc func = nullptr);


		// Observers.

		const vec3& pos() const;

		/* 
		Get entity's postion in X axis.
		*/
		float x() const;

		/*
		Get entity's postion in Y axis.
		*/
		float y() const;

		/*
		Get entity's postion in Z axis.
		*/
		float z() const;

		const vec3& rot() const;

		/*
		Get the entity's model.
		*/
		const model& entityModel() const;

		/*
		Flag to check if model X rotation needs to be updated in the model's vertices.
		*/
		bool updateXRotation() const;

		/*
		Flag to check if model Y rotation needs to be updated in the model's vertices.
		*/
		bool updateYRotation() const;

		/*
		Flag to check if model Z rotation needs to be updated in the model's vertices.
		*/
		bool updateZRotation() const;

		// Get sin(entity's last rotation angle) in X-axis.
		float sinAngleX() const;

		// Get cos(entity's last rotation angle) in X-axis.
		float cosAngleX() const;

		// Get sin(entity's last rotation angle) in Y-axis.
		float sinAngleY() const;

		// Get cos(entity's last rotation angle) in Y-axis.
		float cosAngleY() const;

		// Get sin(entity's last rotation angle) in Z-axis.
		float sinAngleZ() const;

		// Get cos(entity's last rotation angle) in Z-axis.
		float cosAngleZ() const;


		// Modifiers.

		vec3& pos();

		/*
		Set entity's postion in X axis.
		*/ 
		float& x();

		/*
		Set entity's postion in Y axis.
		*/
		float& y();

		/*
		Set entity's postion in Z axis.
		*/
		float& z();

		/*
		Change the entity's model.
		*/
		void setModelID(unsigned int modelID);

		void rotate(const vec3& rotation);

		void rotate(float x, float y, float z);

		/*
		Rotate the entity's model in the X-axis
		*/
		void rotateX(float angle);

		/*
		Rotate the entity's model in the Y-axis
		*/
		void rotateY(float angle);

		/*
		Rotate the entity's model in the Z-axis
		*/
		void rotateZ(float angle);
		
	private:

		/*
		Friend classes.
		*/

		friend entityManager;


		/*
		Attributes.
		*/

		static const float piDiv;

		bool updateXRotation_,
			 updateYRotation_,
			 updateZRotation_;
		vec3 pos_,
			 rot_;
		const model* model_;
		tickFunc tickFunc_;

	};

	inline const vec3& entity::pos() const {
	
		return pos_;
	
	}

	inline const vec3& entity::rot() const {
	
		return rot_;
	
	}

	inline float entity::x() const {
	
		return pos_.x;
	
	}

	inline float entity::y() const {

		return pos_.y;

	}

	inline float entity::z() const {

		return pos_.z;

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

	inline float entity::sinAngleX() const {

		
		return std::sin(rot_.x * piDiv);
	
	}

	inline float entity::cosAngleX() const {

		return std::cos(rot_.x * piDiv);

	}

	inline float entity::sinAngleY() const {

		return std::sin(rot_.y * piDiv);

	}

	inline float entity::cosAngleY() const {

		return std::cos(rot_.y * piDiv);

	}

	inline float entity::sinAngleZ() const {

		return std::sin(rot_.z * piDiv);

	}

	inline float entity::cosAngleZ() const {

		return std::cos(rot_.z * piDiv);

	}

	inline vec3& entity::pos() {

		return pos_;

	}

	inline float& entity::x() {

		return pos_.x;

	}

	inline float& entity::y() {

		return pos_.y;

	}

	inline float& entity::z() {

		return pos_.z;

	}

	inline void entity::setModelID(unsigned int modelID) {
	
		model_ = &models::getModelAt(modelID);
	
	}


	class entityManager {

	public:

		// Initialisation.

		static void init();


		// Observers.

		/*
		An entityID that belongs to a deleted entity does not count
		as an registered entity until a new call to any overload of entityManager::registerEntity()
		reuses said ID (and returns it as the call's result) to create another entity.
		*/
		static bool isEntityRegistered(unsigned int entityID);

		static bool isEntityActiveAt(unsigned int entityID);

		static bool isEntityActive(unsigned int entityID);


		// Modifiers.

		/*
		Function used by a worker thread to manage entities in a world/level.
		Management includes creating, updating and removing entities from the world.
		*/
		static void manageEntities(std::atomic<double>& timeStep);

		/*
		Method called by a batch automatically while it is being constructed to register itself
		in the entityManager's batch list. This allows the manager to access to all the entities in the world.
		Batches are used to save drawing calls, but the game logic for the entities is calculated by the method
		entityManager::manager.
		Returns the ID of the registered batch.
		*/
		static unsigned int registerBatch();

		/*
		Return the total number of existing entities.
		*/
		static unsigned int nEntities();

		/*
		Registers a new entity in the entity management system.
		Returns the ID of the registered entity.
		*/
		static unsigned int registerEntity(unsigned int modelID, const vec3& pos, const vec3& rot = vec3Zero, tickFunc func = nullptr);

		static unsigned int registerEntity(unsigned int modelID, int posX, int posY, int posZ, const vec3& rot = vec3Zero, tickFunc func = nullptr);

		static unsigned int registerEntity(unsigned int modelID, int posX, int posY, int posZ, float rotX, float rotY, float rotZ, tickFunc func = nullptr);

		static entity& getEntityAt(unsigned int ID);

		static entity& getEntity(unsigned int ID);

		static void changeEntityActiveStateAt(unsigned int ID, bool active);

		/*
		Deletes an existing entity from the entity management system.
		The entity ID remains free to be reused for other new entities.
		*/
		static void deleteEntityAt(unsigned int ID);

		/*
		Deletes an existing entity from the entity management system.
		The entity ID remains free to be reused for other new entities.
		*/
		static void deleteEntity(unsigned int ID);

		/*
		Swaps read-only and write-only batch rendering data.
		WARNING. Rendering-thread and entity management thread must be synced when calling this function
		*/
		static void swapReadWrite();

		/*
		Get condition variable used for syncing the rendering and the batching threads.
		*/
		static std::condition_variable& entityManagerCV();

		/*
		Get condition variable flag used for syncing the rendering and the batching threads.
		*/
		static bool syncFlag();

		/*
		Get mutex used for syncing the rendering and the batching threads.
		*/
		static std::mutex& syncMutex();

		/*
		Get the rendering data necessary for the rendering thread to render the batches properly.
		*/
		static const std::vector<const model*>* renderingData();

		
		// Clean up.

		/*
		Called to free dynamic memory allocated by the entity manager system.
		*/
		static void cleanUp();

	private:

		/*
		Attributes.
		*/

		static bool initialised_;
		static std::vector<entity> entities_;
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

		static std::vector<const model*>* renderingDataWrite_,
										* renderingDataRead_;
		
		static std::recursive_mutex entitiesMutex_,
									batchesMutex_;
		static std::condition_variable entityManagerCV_;
		static std::atomic<bool> entityMngCVContinue_;
		static std::mutex syncMutex_;
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


	};

	inline unsigned int entityManager::registerEntity(unsigned int entityTypeID, const vec3& pos, const vec3& rot, tickFunc func) {
	
		return registerEntity(entityTypeID, pos.x, pos.y, pos.z, rot.x, rot.y, rot.z, func);
	
	}

	inline unsigned int entityManager::registerEntity(unsigned int entityTypeID, int posX, int posY, int posZ, const vec3& rot, tickFunc func) {

		return registerEntity(entityTypeID, posX, posY, posZ, rot.x, rot.y, rot.z, func);

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

	inline const std::vector<const model*>* entityManager::renderingData() {
	
		return renderingDataRead_;
	
	}

}

#endif