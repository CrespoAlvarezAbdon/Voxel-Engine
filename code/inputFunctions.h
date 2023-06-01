/**
* @file inputFunctions.h
* @version 1.0
* @date 25/04/2023
* @author Abdon Crespo Alvarez
* @title User input functions.
* @brief The functions that are supposed to be executed when a certain user input
* is received by the engine should at least be declared here.
*/
#ifndef _VOXELENG_INPUTFUNCTIONS_
#define _VOXELENG_INPUTFUNCTIONS_
#include "AIAPI.h"
#include "camera.h"
#include "game.h"
#include "logger.h"


namespace VoxelEng {

	class inputFunctions {

	public:

		// Initialisation.

		/**
		* @brief Initialise the static attributes of this class. This attributes
		* could normally be used as some form of cache of certain data used by
		* the input functions or certains parameters that alter their behaviour.
		* Allocate any resources that are needed on initialisation.
		*/
		static void init();

		// API methods.

		/**
		* @brief Indicate that the user's camera will move in the +y direction according
		* to its movement speed.
		*/
		static void moveUp();

		/**
		* @brief Indicate that the user's camera will move in the -y direction according
		* to its movement speed.
		*/
		static void moveDown();

		/**
		* @brief Indicate that the user's camera will move in the +x direction according
		* to its movement speed.
		*/
		static void moveNorth();

		/**
		* @brief Indicate that the user's camera will move in the -x direction according
		* to its movement speed.
		*/
		static void moveSouth();

		/**
		* @brief Indicate that the user's camera will move in the +z direction according
		* to its movement speed.
		*/
		static void moveEast();
		
		/**
		* @brief Indicate that the user's camera will move in the -z direction according
		* to its movement speed.
		*/
		static void moveWest();

		/**
		* @brief Switch on and off the shader lighting test.
		*/
		static void switchComplexLighting();

		/**
		* @brief Change the recording's play mode to play forward.
		*/
		static void recordForward();

		/**
		* @brief Change the recording's play mode to pause.
	    */
		static void recordPause();

		/**
		* @brief Change the recording's play mode to play backwards.
		*/
		static void recordBackwards();

		/**
		* @brief Exit the current record being played.
		*/
		static void exitRecord();

		/**
		* @brief Select the block 1 as the block the player will place in the level.
		*/
		static void selectBlockSlot1();

		/**
		* @brief Select the block 2 as the block the player will place in the level.
		*/
		static void selectBlockSlot2();

		/**
		* @brief Select the block 3 as the block the player will place in the level.
		*/
		static void selectBlockSlot3();

		/**
		* @brief Select the block 4 as the block the player will place in the level.
		*/
		static void selectBlockSlot4();

		/**
		* @brief Select the block 5 as the block the player will place in the level.
		*/
		static void selectBlockSlot5();

		/**
		* @brief Select the block 6 as the block the player will place in the level.
		*/
		static void selectBlockSlot6();

		/**
		* @brief Select the block 7 as the block the player will place in the level.
		*/
		static void selectBlockSlot7();

		/**
		* @brief Select the block 8 as the block the player will place in the level.
		*/
		static void selectBlockSlot8();

		/**
		* @brief Select the block 9 as the block the player will place in the level.
		*/
		static void selectBlockSlot9();

		/**
		* @brief Intentionally throw an exception to test error management.
		*/
		static void intentionalCrash();

		
		// Clean up.

		/**
		* @brief Free any freeable resources allocated to input functions.
		*/
		static void cleanUp();

	private:

		static bool initialised_;
		static camera* playerCamera_; // To avoid calling game::playerCamera() too many times.

	};

	inline void inputFunctions::moveUp() {

		playerCamera_->moveUp();

	}

	inline void inputFunctions::moveDown() {

		playerCamera_->moveDown();

	}

	inline void inputFunctions::moveNorth() {

		playerCamera_->moveNorth();

	}

	inline void inputFunctions::moveSouth() {

		playerCamera_->moveSouth();

	}

	inline void inputFunctions::moveEast() {

		playerCamera_->moveEast();

	}

	inline void inputFunctions::moveWest() {

		playerCamera_->moveWest();

	}

	inline void inputFunctions::switchComplexLighting() {
	
		game::switchComplexLighting();
	
	}

	inline void inputFunctions::recordForward() {

		AIAPI::aiGame::changeRecordPlayMode(AIAPI::recordPlayMode::FORWARD);

	}

	inline void inputFunctions::recordPause() {

		AIAPI::aiGame::changeRecordPlayMode(AIAPI::recordPlayMode::PAUSE);

	}

	inline void inputFunctions::recordBackwards() {

		AIAPI::aiGame::changeRecordPlayMode(AIAPI::recordPlayMode::BACKWARDS);

	}

	inline void inputFunctions::exitRecord() {
	
		AIAPI::aiGame::stopPlayingRecord();
	
	}

	inline void inputFunctions::selectBlockSlot1() {
	
		player::setBlockToPlace(1);
	
	}

	inline void inputFunctions::selectBlockSlot2() {

		player::setBlockToPlace(2);

	}

	inline void inputFunctions::selectBlockSlot3() {

		player::setBlockToPlace(3);

	}

	inline void inputFunctions::selectBlockSlot4() {

		player::setBlockToPlace(4); 

	}

	inline void inputFunctions::selectBlockSlot5() {

		player::setBlockToPlace(10); // Texture ID 4 is reserved for the warden model's texture.

	}

	inline void inputFunctions::selectBlockSlot6() {

		player::setBlockToPlace(6);

	}

	inline void inputFunctions::selectBlockSlot7() {

		player::setBlockToPlace(7);

	}

	inline void inputFunctions::selectBlockSlot8() {

		player::setBlockToPlace(8);

	}

	inline void inputFunctions::selectBlockSlot9() {

		player::setBlockToPlace(9);

	}

	inline void inputFunctions::intentionalCrash() {
	
		logger::errorLog("This is an intentional error caused to test the engine's error management capabilities.");
	
	}

}

#endif