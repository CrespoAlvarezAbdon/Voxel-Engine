#ifndef _CAMERA_
#define _CAMERA_

#include "definitions.h"
#include "chunk.h"
#include <GLFW/glfw3.h>
#include <gtx/quaternion.hpp>
#include <gtx/rotate_vector.hpp>
#include <glm.hpp>


//////////////////////////////
//Forward class declarations//
//////////////////////////////
class chunk;
class chunkManager;


///////////
//Classes//
///////////

/*
Abstaction of a first person camera.
NOTE. In the future (W.I.P) multiple types of cameras will be added.
*/
class camera
{

public:

	// Constructors.

	camera(float FOV, float width, float height, float zNear, float zFar, GLFWwindow* window,
		   const glm::vec3& position = glm::vec3(0.0f, 0.0f, 0.0f), const glm::vec3& direction = glm::vec3(0.0f, 0.0f, 0.0f));


	// Observers.

	float FOV() const;

	float zFar() const;

	float mouseSensibility() const;

	float movementSpeed() const;

	const glm::mat4& projectionMatrix() const;

	const glm::mat4& viewMatrix() const;

	const glm::vec3& direction() const;

	/*
	Get the camera's chunk relative position.
	*/
	const glm::vec3& chunkPos() const;

	/*
	Get the camera's position.
	*/
	const glm::vec3& pos() const;


	// Modifiers.

	float& mouseSensibility();

	float& movementSpeed();

	glm::mat4& projectionMatrix();

	glm::mat4& viewMatrix();

	glm::vec3& pos();

	glm::vec3& chunkPos();

	void setChunkManager(chunkManager* chunkMng);


	/*
	Update the camera's position and the direction it's looking at
	taking into account the delta time to avoid the FPS from altering the movement speed.
	NOTE. Once this method is called, the next method you should instantly call is
	camera::updateView() to reflect the change in the camera's position and view direction.
	TODO. Update this method to be use GLFW callbacks functions for better input handling.
	*/
	void updatePos(float timeStep);

	/*
	Update the camera's vision.
	NOTE. You should call this method after a call to camera::updatePos(...) was made
	in order to reflect the change in the camera's position and view direction.
	*/
	void updateView();


private:

	float FOV_, 
		  zFar_, 
		  angleX_, 
		  angleY_, 
		  mouseSensibility_, 
		  movementSpeed_;
	double mouseX_, 
		   mouseY_, 
		   oldMouseX_, 
		   oldMouseY_;
	glm::mat4 projectionMatrix_, 
		      viewMatrix_;
	GLFWwindow* window_;
	glm::vec3 position_,
		      direction_, 
		      upAxis_, 
		      chunkRelativePosition_,
		      oldChunkRelativePos_;
	chunkManager* chunkMng_;

};

inline float camera::FOV() const
{

	return FOV_;

}

inline float camera::zFar() const
{

	return zFar_;

}

inline float camera::mouseSensibility() const
{

	return mouseSensibility_;

}

inline float camera::movementSpeed() const
{

	return movementSpeed_;

}

inline const glm::mat4& camera::projectionMatrix() const
{

	return projectionMatrix_;

}

inline const glm::mat4& camera::viewMatrix() const
{

	return viewMatrix_;

}

inline const glm::vec3& camera::pos() const
{

	return position_;

}

inline const glm::vec3& camera::chunkPos() const
{

	return chunkRelativePosition_;

}

inline const glm::vec3& camera::direction() const 
{

	return direction_;

}

inline float& camera::mouseSensibility()
{

	return mouseSensibility_;

}

inline float& camera::movementSpeed()
{

	return movementSpeed_;

}

inline glm::mat4& camera::projectionMatrix()
{

	return projectionMatrix_;

}

inline glm::mat4& camera::viewMatrix()
{

	return viewMatrix_;

}

inline glm::vec3& camera::pos()
{

	return position_;

}

inline glm::vec3& camera::chunkPos()
{

	return chunkRelativePosition_;

}

inline void camera::setChunkManager(chunkManager* chunkMng)
{

	chunkMng_ = chunkMng;

}

inline void camera::updateView()
{

	viewMatrix_ = glm::lookAt(position_, position_ + direction_, upAxis_);

}

#endif