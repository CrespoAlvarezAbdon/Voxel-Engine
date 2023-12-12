/**
* @file tickFunctions.h
* @version 1.0
* @date 25/04/2023
* @author Abdon Crespo Alvarez
* @title Tick functions.
* @brief Contains the declaration of the 'tickFunctions'.
*/
#ifndef _VOXELENG_TICKFUNC_
#define _VOXELENG_TICKFUNC_

#include "AIAPI.h"
#include "entity.h"


namespace VoxelEng {

	//////////////
	//Functions.//
	//////////////

	namespace TickFunctions {
	
		/**
		* @brief This function allows the engine to execute the AI actions stored
		* in the currently opened record file a determined number of times
		* per tick.
		*/
		inline void playRecordTick() {

			AIAPI::aiGame::selectedGame()->playRecordTick();

		}
	
	}

}


#endif