#include "input.h"
#include "gui.h"
#include "game.h"


namespace VoxelEng {

	// 'input' class.

	std::unordered_map<controlCode, void (*)()> input::controlActions_;
	std::unordered_map<controlCode, bool> input::oldActivationEvent_;
	std::recursive_mutex input::inputMutex_;
	window* input::window_ = nullptr;
	bool input::initialised_ = false,
		 input::shouldProcessInputs_ = true;


	void input::init() {

		std::unique_lock<std::recursive_mutex> lock(inputMutex_);

		if (initialised_)
			logger::errorLog("Input system already initialized");
		else {

			window_ = &game::getWindow();
			initialised_ = true;

		}

	}

	void input::setControlAction(controlCode control, void (*action)(), bool isContinuous) {

		if (initialised_) {

			std::unique_lock<std::recursive_mutex> lock(inputMutex_);

			controlActions_[control] = action;

			if (!isContinuous)
				oldActivationEvent_[control] = false;

		}
		else
			logger::errorLog("Initialize input system first");

	}

	void input::handleInputs() {

		std::unique_lock<std::recursive_mutex> lock(inputMutex_);

		bool shouldProcess;
		engineMode loop = game::selectedEngineMode();
		if (loop == engineMode::EDITLEVEL || loop == engineMode::PLAYINGRECORD) {

			// Handle user inputs related to player controls.
			bool activationEventReceived;
			for (auto it = controlActions_.cbegin(); (loop == engineMode::EDITLEVEL ||
				 loop == engineMode::PLAYINGRECORD) && it != controlActions_.cend(); it++) {
			
				activationEventReceived = isControlCodePressed(it->first);
				shouldProcess = input::shouldProcessInputs();

				// This ensures that we only process the action once when the mouse button is pressed.
				if (oldActivationEvent_.find(it->first) == oldActivationEvent_.cend()) {

					if (shouldProcess && activationEventReceived)
						it->second();

				}
				else {
				
					if (shouldProcess && !oldActivationEvent_[it->first] && activationEventReceived) {

						oldActivationEvent_[it->first] = activationEventReceived;
						it->second();

					}
					else
						oldActivationEvent_[it->first] = activationEventReceived;

				}

				loop = game::selectedEngineMode();

			}
				
			// Handle user inputs related to interaction with GUI elements.
			if (loop == engineMode::EDITLEVEL || loop == engineMode::PLAYINGRECORD) 
				GUImanager::processLevelGUIInputs();

		}
		else if (loop == engineMode::GRAPHICALMENU)
			GUImanager::processMainMenuGUIInputs();
	
	}

	void input::shouldProcessInputs(bool newValue) {

		std::unique_lock<std::recursive_mutex> lock(inputMutex_);

		shouldProcessInputs_ = newValue;

	}

	void input::reset() {
	
		controlActions_.clear();
		oldActivationEvent_.clear();
		window_ = nullptr;

		initialised_ = false;
	
	}

}