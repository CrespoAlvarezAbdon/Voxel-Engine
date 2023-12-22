/**
* @file camera.h
* @version 1.0
* @date 20/04/2023
* @author Abdon Crespo Alvarez
* @title Camera.
* @brief Contains the definition of the user's camera in the engine's graphical part.
*/
#ifndef _VOXELENG_CAMERA_
#define _VOXELENG_CAMERA_

#include "chunk.h"
#include "definitions.h"
#include "transform.h"
#include "gameWindow.h"
#include "quaternion.h"
#include "vec.h"

#if GRAPHICS_API == OPENGL

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <gtx/quaternion.hpp>
#include <gtc/quaternion.hpp>
#include <gtx/rotate_vector.hpp>
#include <glm.hpp>

#endif

namespace VoxelEng {


	/////////////////////////
	//Forward declarations.//
	/////////////////////////

	class chunk;
	class chunkManager;
	class window;


	////////////
	//Classes.//
	////////////

	/**
	* @brief Abstraction of an object capable of "viewing" a portion
	* of a simulated 3D world and provide several utilities to
	* later get that vision into a 2D screen. For now it is considered that there
	* is only one camera and that it is attached to the user.
	*/
	class camera {

	public:

		// Constructors.

		/**
		* @brief The class' only constructor.
		* Note: the window parameter must be the one with the graphics API context.
		*/
		camera(float FOV, float zNear, float zFar, window& window, bool isPlayerCamera,
			   const vec3& position = vec3Zero, const vec3& rotation = vec3Zero);


		// Observers.

		/**
		* @brief Provide access to the user's first person camera.
		*/
		static const camera* cPlayerCamera();

		/**
		* @brief Provide access to the user camera's field of view parameter.
		*/
		float FOV() const;

		/**
		* @brief Provide access to the user camera's zFar parameter, which gives the limit
		in the camera's local Z axis at which vertices are no longer included for rendering.
		*/
		float zFar() const;

		/**
		* @brief Provide access to the user camera's movement my mouse sensibility parameter.
		*/
		float mouseSensibility() const;

		/**
		* @brief Provide access to the user camera's movement speed parameter.
		*/
		float movementSpeed() const;

		/**
		* @brief Provide access to the user camera's projection matrix.
		*/
		const glm::mat4& projectionMatrix() const;

		/**
		* @brief Provide access to the user camera's view matrix.
		*/
		const glm::mat4& viewMatrix() const;

		/**
		* @brief Provide access to the user camera's model matrix.
		*/
		const glm::mat4& modelMatrix() const;

		/**
		* @brief Provide access to the user camera's rotation.
		*/
		const vec3& rotation() const;

		/**
		* @brief Get the camera's chunk relative position.
		*/
		const vec3& chunkPos() const;

		/**
		* @brief Get the camera's position.
		*/
		const vec3& pos() const;

		/**
		* @brief Provide access to the user camera's direction.
		*/
		const vec3& viewDirection() const;


		// Modifiers.

		/**
		* @brief Provide access to the user's first person camera.
		*/
		static camera* playerCamera();

		/**
		* @brief Provide access to the user camera's field of view parameter.
		*/
		void setFOV(float FOV);

		/**
		* @brief In case some of the camera's parameters are modified, it will
		be necessary some times to refresh the camera's projection matrix
		(e.g, if the window where the graphics API is rendering the camera's view
		is resized).
		*/
		void updateProjectionMatrix();

		/**
		* @brief Provide access to the camera's movement my mouse sensibility parameter.
		*/
		float& mouseSensibility();

		/**
		* @brief Provide access to the camera's movement speed parameter.
		*/
		float& movementSpeed();

		/**
		* @brief Provide access to the camera's projection matrix.
		*/
		glm::mat4& projectionMatrix();

		/**
		* @brief Provide access to the camera's view matrix.
		*/
		glm::mat4& viewMatrix();

		/**
		* @brief Provide access to the camera's model matrix.
		*/
		glm::mat4& modelMatrix();

		/**
		* @brief Set the camera's position.
		*/
		void setPos(const vec3& newPos);

		/**
		* @brief Set the camera's position.
		*/
		void setPos(float newX, float newY, float newZ);

		/**
		* @brief Set the camera's chunk position.
		*/
		void setChunkPos(const vec3& newChunkPos);

		/**
		* @brief Set the camera's chunk position.
		*/
		void setChunkPos(int newCX, int newCY, int newCZ);

		/**
		* @brief Set the camera's direction.
		*/
		void rotation(const vec3& newRotation);

		/**
		* @brief Set the camera's direction.
		*/
		void rotation(float pitch, float yaw, float roll);

		/**
		* @brief Notify that the camera is to be moved up once according
		* to its movement speed.
		*/
		void moveUp();

		/**
		* @brief Notify that the camera is to be moved down once according
		* to its movement speed.
		*/
		void moveDown();

		/**
		* @brief Notify that the camera is to be moved in global north direction (+x) once according
		* to its movement speed.
		*/
		void moveNorth();

		/**
		* @brief Notify that the camera is to be moved in global south direction (-x) once according
		* to its movement speed.
		*/
		void moveSouth();

		/**
		* @brief Notify that the camera is to be moved in global east direction (+z) once according
		* to its movement speed.
		*/
		void moveEast();

		/**
		* @brief Notify that the camera is to be moved in global west direction (-z) once according
		* to its movement speed.
		*/
		void moveWest();

		/**
		* @brief Notify that the camera will be rolled to its right according to
		* its rolling speed.
		*/
		void rollRight();

		/**
		* @brief Notify that the camera will be rolled to its left according to
		* its rolling speed.
		*/
		void rollLeft();

		/**
		* @brief Update the camera's position and the direction it's looking at
		* taking into account the delta time to avoid the FPS from altering the movement speed.
		* NOTE. Once this method is called, the next method that should be instantly called is
		* camera::updateView() to reflect the change in the camera's position and view direction.
		*/
		void updatePos(float timeStep);

		/**
		* @brief Update the camera's vision.
		* NOTE. This method should be called after one to camera::updatePos(...) was made
		* in order to reflect the change in the camera's position and view direction.
		*/
		void updateView();


		// Destructors.

		/**
		* @brief Class' destructor.
		*/
		~camera();

	private:

		static camera* playerCamera_;

		window& window_;
		bool moveUp_,
			 moveDown_,
			 moveNorth_,
			 moveSouth_,
			 moveEast_,
			 moveWest_,
			 rollRight_,
			 rollLeft_;
		float FOV_,
			  zNear_,
			  zFar_,
			  pitchViewDir_,
			  yawViewDir_,
			  pitch_,
			  yaw_,
			  roll_,
			  mouseSensibility_,
			  movementSpeed_,
			  rollSpeed_;
		double mouseX_,
			   mouseY_,
			   oldMouseX_,
			   oldMouseY_;
		glm::mat4 projectionMatrix_,
				  viewMatrix_,
				  modelMatrix_; // All models' vertices will be multiplied with this matrix (so you can, for example, rotate the entire world around the camera).

		transform transform_;
		vec3 viewDirection_,
			 upAxis_,
			 rightAxis_,
			 forwardAxis_;

		vec3 chunkPosition_,
			 oldChunkPos_;
	};

	inline const camera* camera::cPlayerCamera() {

		return playerCamera_;

	}

	inline float camera::FOV() const {

		return FOV_;

	}

	inline float camera::zFar() const {

		return zFar_;

	}

	inline float camera::mouseSensibility() const {

		return mouseSensibility_;

	}

	inline float camera::movementSpeed() const {

		return movementSpeed_;

	}

	inline const glm::mat4& camera::projectionMatrix() const {

		return projectionMatrix_;

	}

	inline const glm::mat4& camera::viewMatrix() const {

		return viewMatrix_;

	}

	inline const glm::mat4& camera::modelMatrix() const {

		return modelMatrix_;

	}

	inline const vec3& camera::pos() const {

		return transform_.position;

	}

	inline const vec3& camera::viewDirection() const {
	
		return viewDirection_;
	
	}

	inline const vec3& camera::chunkPos() const {

		return chunkPosition_;

	}

	inline const vec3& camera::rotation() const {

		return vec3Zero;

	}

	inline camera* camera::playerCamera() {

		return playerCamera_;

	}

	inline float& camera::mouseSensibility() {

		return mouseSensibility_;

	}

	inline float& camera::movementSpeed() {

		return movementSpeed_;

	}

	inline glm::mat4& camera::projectionMatrix() {

		return projectionMatrix_;

	}

	inline glm::mat4& camera::viewMatrix() {

		return viewMatrix_;

	}

	inline glm::mat4& camera::modelMatrix() {

		return modelMatrix_;

	}

	inline void camera::setPos(const vec3& newPos) {

		setPos(newPos.x, newPos.y, newPos.z);

	}

	inline void camera::setChunkPos(const vec3& newChunkPos) {

		setChunkPos(newChunkPos.x, newChunkPos.y, newChunkPos.z);

	}

	inline void camera::rotation(const vec3& newRotation) {
	
		rotation(newRotation.x, newRotation.y, newRotation.z);
	
	}

	inline void camera::moveUp() {

		moveUp_ = true;

	}

	inline void camera::moveDown() {

		moveDown_ = true;

	}

	inline void camera::moveNorth() {

		moveNorth_ = true;

	}

	inline void camera::moveSouth() {

		moveSouth_ = true;

	}

	inline void camera::moveEast() {

		moveEast_ = true;

	}

	inline void camera::moveWest() {

		moveWest_ = true;

	}

	inline void camera::rollRight() {
	
		rollRight_ = true;
	
	}

	inline void camera::rollLeft() {
	
		rollLeft_ = true;
	
	}

	inline camera::~camera() {
	
		if (this == playerCamera_)
			playerCamera_ = nullptr;
	
	}

}

#endif