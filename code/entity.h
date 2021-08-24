#ifndef _ENTITY_
#define _ENTITY_

#include <glm.hpp>
#include "camera.h"


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
	player(float FOV, float width, float height, float zNear, float zFar, 
		   GLFWwindow* window, const glm::vec3& position = glm::vec3(0.0f, 0.0f, 0.0f), 
		   const glm::vec3& direction = glm::vec3(0.0f, 0.0f, 0.0f));


	// Observers.

	const camera& getCamera() const;


	// Modifiers.

	camera& setCamera();

private:

	camera camera_;

};

inline const camera& player::getCamera() const
{

	return camera_;

}

inline camera& player::setCamera()
{

	return camera_;

}

#endif