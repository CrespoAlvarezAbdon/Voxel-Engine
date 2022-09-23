#ifndef _VOXELENG_TIMER_
#define _VOXELENG_TIMER_
#include <chrono>
#include "definitions.h"


namespace VoxelEng {

	class timer {

	public:

		// Constructors.
		timer();


		// Modifiers.

		/*
		WARNING. A call to start() after another call to the same method will throw an exception.
		The timer starts counting.
		To stop counting, call finish().
		*/
		void start();

		/*
		WARNING. A call to finish() after another call to the same method will throw an exception.
				 A call to finish() without being paired to a call to the start() method will make finish() throw an exception.
		The timer finishes counting.
		A previous call to start() must be made on the same 'timer' object.
		*/
		void clean();

		/*
		Returns elapsed time between the last start() and finish() calls in ms.
		*/
		duration getDurationMs();

	private:

		bool hasStarted_,
			 hasDuration_;
		timePoint tStart_,
				  tEnd_;

	};

}

#endif