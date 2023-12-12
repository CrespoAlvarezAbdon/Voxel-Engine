#ifndef _VOXELENG_LISTENER_
#define _VOXELENG_LISTENER_

#include <unordered_map>
#include "event.h"

namespace VoxelEng {

	/////////////////////////
	//Forward declarations.//
	/////////////////////////

	class event;


	////////////
	//Classes.//
	////////////

	/**
	* @brief Entity that only executes a specific method when an event it is attached
	* to is triggered.
	*/
	class listener {

	public:

		// Modifiers

		/**
		* @brief The method to execute when the attached event occurs.
		*/
		virtual void onEvent(event* e) = 0;

	private:

	};

}

#endif