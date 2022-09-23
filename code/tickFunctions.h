#ifndef _VOXELENG_TICKFUNC_
#define _VOXELENG_TICKFUNC_
#include "AIAPI.h"


namespace VoxelEng {

	//////////////
	//Functions.//
	//////////////

	namespace TickFunctions {
	
		

		inline void playRecordTick() {

			AIAPI::aiGame::selectedGame()->playRecordTick();

		}
	
	}

}


#endif