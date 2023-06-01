/**
* @file timer.h
* @version 1.0
* @date 25/04/2023
* @author Abdon Crespo Alvarez
* @title Timer.
* @brief Contains the declaration of the 'timer' class
*/
#ifndef _VOXELENG_TIMER_
#define _VOXELENG_TIMER_
#include <chrono>
#include <atomic>
#include "definitions.h"

namespace VoxelEng {

	////////////
	//Classes.//
	////////////

	/**
	* @brief Provides the capability of measuring the duration of fragments
	* of the engine's code.
	*/
	class timer {

	public:

		// Constructors.

		/**
		* @brief Class constructor.
		*/
		timer();


		// Observers.

		/**
		* @brief Returns true if the timer has started counting or false otherwise.
		*/
		bool hasStarted() const;


		// Modifiers.

		/**
		* @brief The timer starts counting.
		* To stop counting, call finish().
		* WARNING. A call to start() after another call to the same method will throw an exception.
		*/
		void start();

		/**
		* @brief The timer finishes counting.
		* A previous call to start() must be made on the same 'timer' object.
		* WARNING. A call to finish() after another call to the same method will throw an exception.
		* WARNING. A call to finish() without being paired to a call to the start() method will make finish() throw an exception.
		*/
		void finish();

		/**
		* @brief Returns elapsed time between the last start() and finish() calls in ms.
		*/
		duration getDurationMs();

	private:

		std::atomic<bool> hasStarted_,
						  hasDuration_;
		timePoint tStart_,
				  tEnd_;

	};

	inline bool timer::hasStarted() const {
	
		return hasStarted_;
	
	}

}

#endif