#ifndef _ENTITY_
#define _ENTITY_

#include "camera.h"
#include "chunk.h"
#include "gameWindow.h"
#include <GLFW/glfw3.h>
#include <glm.hpp>


///////////
//Classes//
///////////

/*
Abstraction that contains everything that defines the player.
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

		// Modifiers.

		// Set entity's postion in X axis.
		float& x();
		// Set entity's postion in Y axis.
		float& y();
		// Set entity's postion in Z axis.
		float& z();


	private:

		float x_, y_, z_;
		model model_;

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

	inline float& entity::x() {

		return x_;

	}

	inline float& entity::y() {

		return y_;

	}

	inline float& entity::z() {

		return z_;

	}

}

#endif