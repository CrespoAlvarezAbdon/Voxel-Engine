#include <string>
#include <filesystem>
#include "chunk.h"
#include "gui.h"
#include "graphics.h"
#include "GUIFunctions.h"
#include "logger.h"


namespace VoxelEng {

	namespace GUIFunctions {

		void changeStateMainMenu() {

			GUIManager::changeGUIState("mainMenu");
			if (game::loopSelection() == GRAPHICALLEVEL)
				GUIManager::changeGUIState("mainMenu.saveButton");
			else {

				GUIManager::changeGUIState("mainMenu.newButton");
				GUIManager::changeGUIState("mainMenu.loadButton");

			}
			GUIManager::changeGUIState("mainMenu.exitButton");

		}

		void changeStateLevelMenu() {

			if (game::loopSelection() == GRAPHICALLEVEL) {
			
				GUIManager::switchLevelGUIOpened();
				changeStateMainMenu();
				graphics::getMainWindow()->changeStateMouseLock();
			
			}

		}

		void changeStateMainMenu(bool isEnabled) {
		
			GUIManager::changeGUIState("mainMenu", isEnabled);
			GUIManager::changeGUIState("mainMenu.loadButton", isEnabled);
			if (game::loopSelection() == GRAPHICALLEVEL)
				GUIManager::changeGUIState("mainMenu.saveButton", isEnabled);
			else {

				GUIManager::changeGUIState("mainMenu.newButton", isEnabled);
				GUIManager::changeGUIState("mainMenu.loadButton", isEnabled);

			}
			GUIManager::changeGUIState("mainMenu.exitButton", isEnabled);
		
		}

		void showLoadMenu() {

			game::setSlotAccessType(slotAccessType::load);

			changeStateMainMenu(false);
			GUIManager::changeGUIState("loadMenu", true);
			GUIManager::changeGUIState("loadMenu.exitButton", true);

			for (int i = 1; i <= 5; i++)
				GUIManager::changeGUIState("saveSlot" + std::to_string(i), true);

		}

		void hideLoadMenu() {

			GUIManager::changeGUIState("loadMenu", false);
			GUIManager::changeGUIState("loadMenu.exitButton", false);

			for (int i = 1; i <= 5; i++)
				GUIManager::changeGUIState("saveSlot" + std::to_string(i), false);

			changeStateMainMenu(true);

		}

		void showSaveMenu() {

			game::setSlotAccessType(slotAccessType::save);

			changeStateMainMenu(false);
			GUIManager::changeGUIState("saveMenu", true);
			GUIManager::changeGUIState("saveMenu.exitButton", true);

			for (int i = 1; i <= 5; i++)
				GUIManager::changeGUIState("saveSlot" + std::to_string(i), true);

		}

		void hideSaveMenu() {

			GUIManager::changeGUIState("saveMenu", false);
			GUIManager::changeGUIState("saveMenu.exitButton", false);

			for (int i = 1; i <= 5; i++)
				GUIManager::changeGUIState("saveSlot" + std::to_string(i), false);

			changeStateMainMenu(true);

		}

		void saveGame() {

			// Convert character to int number (assuming the last character of the GUIElement's name is a digit).
			unsigned int saveSlot = GUIManager::lastCheckedGUIElement()->name().back() - '0';
			game::setSaveSlot(saveSlot);

			// Save chunk data into selected save slot.
			chunkManager::saveAllChunks("saves/slot" + std::to_string(saveSlot) + "/level.terrain");

			logger::debugLog("Saved on slot " + std::to_string(saveSlot));


		}

		void loadGame() {

			// Convert character to int number (assuming the last character of the GUIElement's name is a digit).
			unsigned int saveSlot = GUIManager::lastCheckedGUIElement()->name().back() - '0';
			game::setSaveSlot(saveSlot);

			// Check if save slot has valid data written.
			if (std::filesystem::exists(std::filesystem::path("saves/slot" + std::to_string(game::selectedSaveSlot()) + "/level.terrain"))) {

				// Hide load menu and don't show the main menu like the hideLoadMenu() function.
				GUIManager::changeGUIState("loadMenu", false);
				GUIManager::changeGUIState("loadMenu.exitButton", false);

				for (int i = 1; i <= 5; i++)
					GUIManager::changeGUIState("saveSlot" + std::to_string(i), false);

				game::enterLevel();

				logger::debugLog("Selected to load slot " + std::to_string(saveSlot));
			
			}
			else
				logger::debugLog("The selected slot is empty");

		}

		void enterNewLevel() {
		
			GUIManager::setLevelGUIOpened(false);

			changeStateMainMenu(false);

			game::enterLevel();
			game::setSaveSlot(0);
			
		}

		void accessSaveSlot() {
		
			if (game::getSlotAccessType() == slotAccessType::load)
				loadGame();
			else
				saveGame();
		
		}

		void exit() {
		
			switch (game::loopSelection()) {
			
			case AIMENULOOP:

				game::cleanUp();

				break;

			case GRAPHICALMENU:

				game::goToAIMenu();

				break;

			case GRAPHICALLEVEL:

				game::goToGraphicalMenu();

				break;
			
			}
		
		}

	}

}