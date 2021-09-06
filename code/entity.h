#ifndef _ENTITY_
#define _ENTITY_

#include "camera.h"
#include "chunk.h"
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
	player(float FOV, float width, float height, float zNear, float zFar, GLFWwindow* window, 
		   unsigned int blockReachRange, const glm::vec3& position = glm::vec3(0.0f, 0.0f, 0.0f),
		   const glm::vec3& direction = glm::vec3(0.0f, 0.0f, 0.0f));


	// Observers.

	const camera& mainCamera() const;


	// Modifiers.

	camera& mainCamera();

	void setChunkManager(chunkManager* chunkMng);


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

private:

	GLFWwindow* window_;
	camera camera_;
	float blockReachRange_,
		  blockSearchIncrement_;
	chunkManager* chunkMng_;
	block selectedBlock_;
	glm::vec3 selectedBlockPos_,
			  oldSelectedBlockPos_;

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

inline void player::mouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
{

	static_cast<player*>(glfwGetWindowUserPointer(window))->mouseButtonHandler(window, button, action, mods);

}

#endif