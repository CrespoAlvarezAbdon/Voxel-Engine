#ifndef _VOXELENG_INPUTFUNCTIONS_
#define _VOXELENG_INPUTFUNCTIONS_
#include "camera.h"
#include "game.h"


namespace VoxelEng {

	class inputFunctions {

	public:

		// Initialisation.

		static void init();

		// API methods.

		static void moveUp(); // +y

		static void moveDown(); // -y

		static void moveNorth(); // +x

		static void moveSouth(); // -x

		static void moveEast(); // +z

		static void moveWest(); // -z

		static void switchComplexLighting();

		// TODO: IMPLEMENT.
		/*static void selectBlockSlot1();

		static void selectBlockSlot2();

		static void selectBlockSlot3();

		static void selectBlockSlot4();

		static void selectBlockSlot5();

		static void selectBlockSlot6();

		static void selectBlockSlot7();

		static void selectBlockSlot8();

		static void selectBlockSlot9();*/

		
		// Clean up.

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

}

#endif