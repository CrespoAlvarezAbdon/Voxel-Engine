#ifndef _VOXELENG_CAMERA_
#define _VOXELENG_CAMERA_
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <gtx/quaternion.hpp>
#include <gtx/rotate_vector.hpp>
#include <glm.hpp>
#include "definitions.h"
#include "chunk.h"
#include "gameWindow.h"


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

	class camera {

	public:

		// Constructors.

		camera(float FOV, float zNear, float zFar, window& window, bool isPlayerCamera,
			const vec3& position = vec3(0.0f, 0.0f, 0.0f), const vec3& direction = vec3(0.0f, 0.0f, 0.0f));


		// Observers.

		static const camera* cPlayerCamera();

		float FOV() const;

		float zFar() const;

		float mouseSensibility() const;

		float movementSpeed() const;

		const glm::mat4& projectionMatrix() const;

		const glm::mat4& viewMatrix() const;

		const glm::mat4& modelMatrix() const;

		const vec3& direction() const;

		/*
		Get the camera's chunk relative position.
		*/
		const vec3& chunkPos() const;

		/*
		Get the camera's position.
		*/
		const vec3& pos() const;


		// Modifiers.

		static camera* playerCamera();

		void setFOV(float FOV);

		void updateProjectionMatrix();

		float& mouseSensibility();

		float& movementSpeed();

		glm::mat4& projectionMatrix();

		glm::mat4& viewMatrix();

		glm::mat4& modelMatrix();

		void setPos(const vec3& newPos);

		void setPos(int newX, int newY, int newZ);

		void setChunkPos(const vec3& newChunkPos);

		void setChunkPos(int newCX, int newCY, int newCZ);

		void setDirection(const vec3& newDir);

		void setDirection(int newDX, int newDY, int newDZ);

		void moveUp();

		void moveDown();

		void moveNorth();

		void moveSouth();

		void moveEast();

		void moveWest();

		/*
		Update the camera's position and the direction it's looking at
		taking into account the delta time to avoid the FPS from altering the movement speed.
		NOTE. Once this method is called, the next method that should be instantly called is
		camera::updateView() to reflect the change in the camera's position and view direction.
		*/
		void updatePos(float timeStep);

		/*
		Update the camera's vision.
		NOTE. This method should be called after one to camera::updatePos(...) was made
		in order to reflect the change in the camera's position and view direction.
		*/
		void updateView();


		// Destructors.

		~camera();

	private:

		static camera* playerCamera_;

		window& window_;
		bool moveUp_,
			 moveDown_,
			 moveNorth_,
			 moveSouth_,
			 moveEast_,
			 moveWest_;
		float FOV_,
			 zNear_,
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
				  viewMatrix_,
				  modelMatrix_; // All models' vertices will be multiplied with this matrix (so you can, for example, rotate the entire world around the camera).

		vec3 position_,
		     direction_,
			 upAxis_,
			 chunkPosition_,
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

		return position_;

	}

	inline const vec3& camera::chunkPos() const {

		return chunkPosition_;

	}

	inline const vec3& camera::direction() const {

		return direction_;

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

	inline void camera::setDirection(const vec3& newDir) {
	
		setDirection(newDir.x, newDir.y, newDir.z);
	
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

	inline void camera::updateView() {

		#if GRAPHICS_API == OPENGL

			viewMatrix_ = glm::lookAt(position_, position_ + direction_, upAxis_);

		#else

			

		#endif

	}

	inline camera::~camera() {
	
		if (this == playerCamera_)
			playerCamera_ = nullptr;
	
	}

}

#endif