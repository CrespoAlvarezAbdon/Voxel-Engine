#include "input.h"
#include "gui.h"
#include "game.h"


namespace VoxelEng {

	// 'input' class.

	std::unordered_map<controlCode, void (*)()> input::controlActions_;
	std::unordered_map<controlCode, bool> input::oldActivationEvent_;
	std::mutex input::inputMutex_;
	window* input::window_ = nullptr;
	bool input::initialised_ = false;


	void input::init() {

		std::unique_lock<std::mutex> lock(inputMutex_);

		if (initialised_)
			logger::errorLog("Input system already initialized");
		else {

			window_ = &game::getWindow();
			initialised_ = true;

		}

	}

	void input::setControlAction(controlCode control, void (*action)(), bool isContinuous) {

		if (initialised_) {

			std::unique_lock<std::mutex> lock(inputMutex_);

			controlActions_[control] = action;

			if (!isContinuous)
				oldActivationEvent_[control] = false;

		}
		else
			logger::errorLog("Initialize input system first");

	}

	void input::handleInputs() {

		std::unique_lock<std::mutex> lock(inputMutex_);

		if (game::loopSelection() == GRAPHICALLEVEL) {

			// Handle user inputs related to player controls.
			bool activationEventReceived;
			for (auto it = controlActions_.cbegin(); it != controlActions_.cend(); it++) {
			
				activationEventReceived = isControlCodePressed(it->first);

				// This ensures that we only process the action once when the mouse button is pressed.
				if (oldActivationEvent_.find(it->first) == oldActivationEvent_.cend()) {

					if (activationEventReceived)
						it->second();

				}
				else {
				
					if (!oldActivationEvent_[it->first] && activationEventReceived) {

						oldActivationEvent_[it->first] = activationEventReceived;
						it->second();

					}
					else
						oldActivationEvent_[it->first] = activationEventReceived;

				}

			}
				
			// Handle user inputs related to interaction with GUI elements.
			GUIManager::processLevelGUIInputs();

		}
		else
			GUIManager::processMainMenuGUIInputs();
	
	}

	void input::cleanUp() {
	
		controlActions_.clear();
		oldActivationEvent_.clear();
		window_ = nullptr;

		initialised_ = false;
	
	}

}