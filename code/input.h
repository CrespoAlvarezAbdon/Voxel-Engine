#ifndef _VOXELENG_INPUT_
#define _VOXELENG_INPUT_
#include <unordered_map>
#include <mutex>
#include "controls.h"
#include "gameWindow.h"


namespace VoxelEng {

	////////////
	//Classes.//
	////////////

	class input {

	public:

		// Initialisation.

		static void init();


		// Modifiers.

		/*
		This operation is thread-safe.
		*/
		static void setControlAction(controlCode code, void (*action)(), bool isContinuous = true);

		static void handleInputs();


		// Clean up.

		static void cleanUp();

	private:

		// Attributes.

		static std::unordered_map<controlCode, void (*)()> controlActions_;
		static std::unordered_map<controlCode, bool> oldActivationEvent_;
		static std::mutex inputMutex_;
		static window* window_;
		static bool initialised_;

		// Methods.

		/*
		It's a private method because all user inputs must be handled with void (*)() functions
		that are registered and associated with a controlCode with setControlAction() method and
		executed with handleInputs() method. This ensures that the code remains clear and organized.
		*/
		static bool isControlCodePressed(controlCode code);

	};

	inline bool input::isControlCodePressed(controlCode code) {

		return glfwGetKey(window_->windowAPIpointer(), controls::getGLFWControlCode(code)) == GLFW_PRESS;

	}

}

#endif