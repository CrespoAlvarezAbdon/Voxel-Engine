/**
* @file player.h
* @version 1.0
* @date 23/12/2023
* @author Abdon Crespo Alvarez
* @title Player.
* @brief Contains the declaration of the Player class, an especial case of entity that represents
* the user in the world.
*/
#ifndef _VOXELENG_PLAYER_
#define _VOXLENEG_PLAYER_

#include "block.h"
#include "entity.h"
#include "gameWindow.h"
#include "transform.h"
#include "vec.h"

#if GRAPHICS_API == OPENGL

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm.hpp>

#endif

namespace VoxelEng {

	/**
	* @brief Abstraction containing everything that defines the user's entity in the world.
	*/
	class player {

	public:

		// Initialisation.

		/**
		* @brief Initialise the player system and all the class' static attributes.
		*/
		static void init(float FOV, float zNear, float zFar, window& window, unsigned int blockReachRange);


		// Observers.

		static const vec3& globalPos();

		static const vec3& rotation();


		// Modifiers.

		/**
		* @brief Get the user's first person camera.
		*/
		static camera& getCamera();

		/**
		* @brief Computes a basic raycast to select a block in the world that is
		* reachable from where the user stands.
		*/
		static void selectBlock();

		/**
		* @brief Callback function to get the user's mouse buttons input.
		*/
		static void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods);

		/**
		* @brief Destroys the block selected by the user's camera, if any.
		*/
		static void destroySelectedBlock();

		/**
		* @brief Places the current selected block where the user is looking at.
		* Cannot place a block on thin air, must be looking into a solid block to place.
		*/
		static void placeSelectedBlock();

		/**
		* @brief Function to process user's raycast used to interact with the level's enviroment.
		* WARNING. Must not be called in the rendering thread.
		*/
		static void processSelectionRaycast();

		/**
		* @brief Change the current block ID used by the player to place blocks.
		*/
		static void setBlockToPlace(block& block);

		/**
		* @brief Change user position and viewing direction.
		*/
		static void changeTransform(const vec3& newPos, const vec3& newRot = vec3Zero);

		/**
		* @brief Change user position and viewing direction.
		*/
		static void changeTransform(float newX, float newY, float newZ, float pitch = 0.0f, float yaw = 0.0f, float roll = 0.0f);

		/**
		* @brief Notify that the camera is to be moved up once according
		* to its movement speed.
		*/
		static void moveUp();

		/**
		* @brief Notify that the camera is to be moved down once according
		* to its movement speed.
		*/
		static void moveDown();

		/**
		* @brief Notify that the camera is to be moved in global north direction (+x) once according
		* to its movement speed.
		*/
		static void moveNorth();

		/**
		* @brief Notify that the camera is to be moved in global south direction (-x) once according
		* to its movement speed.
		*/
		static void moveSouth();

		/**
		* @brief Notify that the camera is to be moved in global east direction (+z) once according
		* to its movement speed.
		*/
		static void moveEast();

		/**
		* @brief Notify that the camera is to be moved in global west direction (-z) once according
		* to its movement speed.
		*/
		static void moveWest();

		/**
		* @brief Notify that the camera will be rolled to its right according to
		* its rolling speed.
		*/
		static void rollRight();

		/**
		* @brief Notify that the camera will be rolled to its left according to
		* its rolling speed.
		*/
		static void rollLeft();

		/**
		* @brief Update the player's transforms.
		* Called on the rendering thread while applying the time step in order to
		* un-link the frame rate with the player's movement.
		* This will also update the associated camera's transform.
		*/
		static void updateTransform(float timeStep);


		// Clean up.

		/**
		* @brief Used to clean up the heap memory allocated by this system and deinitialise it.
		*/
		static void reset();

	private:

		static bool initialised_,
					moveUp_,
					moveDown_,
					moveNorth_,
					moveSouth_,
					moveEast_,
					moveWest_,
					rollRight_,
					rollLeft_;
		static GLFWwindow* window_;
		static camera* camera_;
		static float blockReachRange_,
					 blockSearchIncrement_,
					 movementSpeed_,
			         rollSpeed_;
		static const block* selectedBlock_;
		static std::atomic<const block*> blockToPlace_;
		static vec3 selectedBlockPos_,
					oldSelectedBlockPos_;
		static entity* playerEntity_;
		static transform* playerTransform_;

		/*
		Flags used to coordinate the callbacks called
		on the rendering thread with the input processing thread.
		*/
		static std::atomic<bool> destroyBlock_,
								 placeBlock_;

	};

	inline const vec3& player::globalPos() {

		return camera_->globalPos();

	}

	inline const vec3& player::rotation() {

		return camera_->rotation();

	}

	inline camera& player::getCamera() {

		return *camera_;

	}

	inline void player::changeTransform(const vec3& newPos, const vec3& newRot) {

		changeTransform(newPos.x, newPos.y, newPos.z, newRot.x, newRot.y, newRot.z);

	}

	inline void player::moveUp() {

		moveUp_ = true;

	}

	inline void player::moveDown() {

		moveDown_ = true;

	}

	inline void player::moveNorth() {

		moveNorth_ = true;

	}

	inline void player::moveSouth() {

		moveSouth_ = true;

	}

	inline void player::moveEast() {

		moveEast_ = true;

	}

	inline void player::moveWest() {

		moveWest_ = true;

	}

	inline void player::rollRight() {

		rollRight_ = true;

	}

	inline void player::rollLeft() {

		rollLeft_ = true;

	}

}

#endif