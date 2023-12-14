/**
* @file input.h
* @version 1.0
* @date 25/04/2023
* @author Abdon Crespo Alvarez
* @title User input.
* @brief Provides a class that manages and provides all the aspects related to how
* the engine accepts and process user input from keyboard and mouse (except standard input).
* In the case of user input that affects GUIelements, the GUIfunctions and the GUImanager
* are responsible for managing those cases.
*/
#ifndef _VOXELENG_INPUT_
#define _VOXELENG_INPUT_

#include <atomic>
#include <unordered_map>
#include <mutex>
#include "controls.h"
#include "gameWindow.h"


namespace VoxelEng {

	////////////
	//Classes.//
	////////////

	/**
	* @brief Responsible for managing and providing all aspects related
	* to how the engine accepts and process the user input from keyboard
	* and mouse (except standard input).
	* In the case of user input that affects GUIelements, the GUIfunctions and the GUImanager
	* are responsible for managing those cases.
	*/
	class input {

	public:

		// Initialisation.

		/**
		* @brief Initialise the input system.
		* Allocate any resources that are needed on initialisation.
		*/
		static void init();


		// Observers.

		/**
		* @brief Returns true if the input system is currently accepting any type of
		* user input or false otherwise.
		* WARNING. Lock input::inputMutex() if it is not desirable for this flag to be 
		* altered by other thread. Access without doing this does not ensure mutual exclusion!
		*/
		static bool shouldProcessInputs();


		// Modifiers.

		/**
		* @brief Assign a control code with an executed action.
		* Note.This operation is thread-safe.
		*/
		static void setControlAction(controlCode code, void (*action)(), bool isContinuous = true);

		/**
		* @brief Method to handle the user inputs in one game's tick.
		*/
		static void handleInputs();

		/**
		* @ brief Returns the mutex associated with the user input system.
		*/
		static std::recursive_mutex& inputMutex();

		/**
		* @brief Set the value of the flag that indicates if the input system 
		* is currently accepting any type of user input or not.
		* WARNING. Locks input::inputMutex().
		*/
		static void shouldProcessInputs(bool newValue);


		// Clean up.

		/**
		* @brief Deinitialise the input system and clean up any resources (heap memory, opened files...)
		* allocated to it.
		*/
		static void reset();

	private:

		/*
		Attributes.
		*/

		static std::unordered_map<controlCode, void (*)()> controlActions_;
		static std::unordered_map<controlCode, bool> oldActivationEvent_; // If the specified controlCode was triggered in the last iteration
		static std::recursive_mutex inputMutex_;
		static window* window_;
		static bool initialised_,
			        shouldProcessInputs_;


		/*
		Methods.
		*/

		// Observers.

		/*
		All user inputs must be handled with void (*)() functions
		that are registered and associated with a controlCode with setControlAction() method and
		executed with handleInputs() method. This ensures that the code remains clear and organized.
		*/
		static bool isControlCodePressed(controlCode code);

	};

	inline std::recursive_mutex& input::inputMutex() {
	
		return inputMutex_;
	
	}

	inline bool input::shouldProcessInputs() {
	
		return shouldProcessInputs_;
	
	}

	inline bool input::isControlCodePressed(controlCode code) {

		return glfwGetKey(window_->windowAPIpointer(), controls::getGLFWControlCode(code)) == GLFW_PRESS;

	}

}

#endif