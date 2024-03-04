/**
* @file GUIfunctions.h
* @version 1.0
* @date 25/04/2023
* @author Abdon Crespo Alvarez
* @title GUIfunctions.
* @brief Contains the declaration of the GUIfunctions, which are functions that are related to
* altering or accesing any aspect of the engine's GUI.
*/
#ifndef _VOXELENG_GUIFUNCTIONS_
#define _VOXELENG_GUIFUNCTIONS_

#include "game.h"


namespace VoxelEng {

	namespace GUIfunctions {

		/**
		* @brief Change the active state of the in-level menu.
		*/
		void changeStateLevelMenu();

		/**
		* @brief Show the level slot load menu.
		*/
		void showLoadMenu();

		/**
		* @brief Hide the level slot load menu.
		*/
		void hideLoadMenu();

		/**
		* @brief Save all the data regarding the currently opened save slot
		* (including chunks).
		*/
		void saveGame();

		/**
		* @brief Load the selected save slot.
		* WARNING. Must be called from a save slot button.
		*/
		void loadGame();

		/**
		* @brief Proceed to generate a new level and load it.
		*/
		void enterNewLevel();

		/**
		* @brief Exit into the upper level of execution of the engine. For example, if the engine is currently
		* with the graphical part turned ON and with a level loaded and rendered with the user editing it and
		* this function is called, then the engine will return to the graphical main menu. If the function
		* is called again, the engine will turn off its graphical part and display the console menu.
		* Finally, if the function is executed another time, then the engine will end its execution.
		*/
		void exit();

	}

}

#endif