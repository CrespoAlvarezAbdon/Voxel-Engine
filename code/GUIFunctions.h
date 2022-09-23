#ifndef _VOXELENG_GUIFUNCTIONS_
#define _VOXELENG_GUIFUNCTIONS_
#include "game.h"


namespace VoxelEng {

	namespace GUIFunctions {

		void changeStateLevelMenu();

		void showLoadMenu();

		void hideLoadMenu();

		void showSaveMenu();

		void hideSaveMenu();

		void enterNewLevel();

		/*
		Access the corresponding save slot associated with the GUIButton that called this function
		with the set slot access type specified in the 'game' class.
		*/
		void accessSaveSlot();

		void exit();

	}

}

#endif