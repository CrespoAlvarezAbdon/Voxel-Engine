#ifndef _VOXENG_ENTITY_
#define _VOXENG_ENTITY_

#include "camera.h"
#include "chunk.h"
#include "gameWindow.h"
#include "batch.h"
#include "model.h"
#include "gameWindow.h"
#include <vector>
#include <deque>
#include <mutex>
#include <atomic>
#include <condition_variable>
#include <unordered_map>
#include <GLFW/glfw3.h>
#include <glm.hpp>


///////////
//Classes//
///////////

/*
Abstraction that contains everything that defines the player (NOT the player model/entity).
*/
class player
{

public:

	// Constructors.

	/*
	Spawns the player in the world and initialices its corresponding camera.
	*/
	player(float FOV, float zNear, float zFar, VoxelEng::window& window,
		   unsigned int blockReachRange, const glm::vec3& position, unsigned int spawnWorldID,
		   atomic<bool>* appFinished, const glm::vec3& direction = glm::vec3(0.0f, 0.0f, 0.0f));


	// Observers.

	const camera& mainCamera() const;


	// Modifiers.

	camera& mainCamera();

	void setChunkManager(chunkManager* chunkMng);


	// Other methods.

	/*
	Fakes a raycast to select a block in the world that is
	reachable from where the player stands.
	*/
	void selectBlock();

	/*
	Callback function to get the player's mouse buttons
	input.
	*/
	static void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods);

	/*
	Handle all player input related to mouse buttons.
	*/
	void mouseButtonHandler(GLFWwindow* window, int button, int action, int mods);

	/*
	Destroys the block selected by the player's camera, if any.
	*/
	void destroySelectedBlock();

	/*
	Places the current selected block in the hotbar (W.I.P) where the player is looking at.
	Cannot place a block on thin air, must be looking into a solid block to place.
	*/
	void placeSelectedBlock();

	/*
	Function to process player input.
	WARNING. Must not be called in the rendering thread.
	*/
	void processPlayerInput();

private:

	GLFWwindow* window_;
	camera camera_;
	float blockReachRange_,
		  blockSearchIncrement_;
	chunkManager* chunkMng_;
	VoxelEng::block selectedBlock_;
	glm::vec3 selectedBlockPos_,
			  oldSelectedBlockPos_;

	/*
	Flags used to coordinate the callbacks called
	on the rendering thread with the input processing thread.
	*/
	atomic<bool> destroyBlock_,
			     placeBlock_;

	atomic<bool>* appFinished_;

};

inline const camera& player::mainCamera() const
{

	return camera_;

}

inline camera& player::mainCamera()
{

	return camera_;

}

inline void player::setChunkManager(chunkManager* chunkMng)
{

	chunkMng_ = chunkMng;

}

namespace VoxelEng {

	//////////////////////////////
	//Forward class declarations//
	//////////////////////////////
	class batch;

	///////////
	//Classes//
	///////////

	class entity {

	public:

		// Constructors.

		entity(unsigned int modelID, float x, float y, float z);


		// Observers.

		// Get entity's postion in X axis.
		float x() const;
		// Get entity's postion in Y axis.
		float y() const;
		// Get entity's postion in Z axis.
		float z() const;

		// Get the entity's model.
		const model& entityModel() const;

		// Get entity's last modified rotational axis.
		unsigned int lastRotAxis() const;

		// Get sin(entity's last rotation angle).
		float sinAngle() const;

		// Get cos(entity's last rotation angle).
		float cosAngle() const;


		// Modifiers.

		// Set entity's postion in X axis.
		float& x();
		// Set entity's postion in Y axis.
		float& y();
		// Set entity's postion in Z axis.
		float& z();

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

		unsigned int axis_; // Te dice en qué eje debes rotar
		float x_, y_, z_, sinAngle_, cosAngle_;
		const model& model_;

	};

	inline float entity::x() const {
	
		return x_;
	
	}

	inline float entity::y() const {

		return y_;

	}

	inline float entity::z() const {

		return z_;

	}

	inline const model& entity::entityModel() const {

		return model_;

	}

	inline unsigned int entity::lastRotAxis() const {
	
		return axis_;
	
	}

	inline float entity::sinAngle() const {
	
		return sinAngle_;
	
	}

	inline float entity::cosAngle() const {

		return cosAngle_;

	}

	inline float& entity::x() {

		return x_;

	}

	inline float& entity::y() {

		return y_;

	}

	inline float& entity::z() {

		return z_;

	}

	class entityManager {

	public:

		/*
		Function used by a worker thread to manage entities in a world/level.
		Management includes creating, updating and removing entities from the world.
		*/
		static void manageEntities(atomic<double>& timeStep, const atomic<bool>& appFinished);

		/*
		Method called by a batch automatically while it is being constructed to register itself
		in the entityManager's batch list. This allows the manager to access to all the entities in the world.
		Batches are used to save drawing calls, but the game logic for the entities is calculated by the method
		entityManager::manager.
		Returns the ID of the registered batch.
		*/
		static unsigned int registerBatch(batch* batch);

		/*
		Return the total number of existing entities.
		*/
		static unsigned int nEntities();

		/*
		Registers a new entity in the entity management system.
		*/
		static void registerEntity(entity& entity);

		/*
		Deletes an existing entity from the entity management system.
		*/
		static void removeEntity(unsigned int ID);

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

	private:

		static std::vector<batch*> batches_;
		static std::vector<const model*>* renderingDataWrite_,
								        * renderingDataRead_;
		static std::deque<unsigned int> freeBatchInd_,
										freeEntityID_;
		static std::unordered_map<unsigned int, entity*> entityIDList_;
		static std::unordered_map<unsigned int, unsigned int> entityBatch_; // Relates entity's ID with the batch it belongs to batch.
		
		static std::recursive_mutex entityIDListMutex_,
			                        batchesMutex_;
		static std::condition_variable entityManagerCV_;
		static std::atomic<bool> entityMngCVContinue_;
		static std::mutex syncMutex_;

	};

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