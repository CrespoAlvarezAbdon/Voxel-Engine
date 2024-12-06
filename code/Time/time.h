/**
* @file time.h
* @version 1.0
* @date 25/04/2023
* @author Abdon Crespo Alvarez
* @title Time.
* @brief Contains the declaration of the 'time' class and other time-related
* utilities.
*/
#ifndef _VOXELENG_TIME_
#define _VOXELENG_TIME_

#include "definitions.h"

#if GRAPHICS_API == OPENGL

	#include <GL/glew.h>
	#include <GLFW/glfw3.h>

#else

#endif


namespace VoxelEng {

	////////////////
	//Enumerators.//
	////////////////

	/**
	* @brief The different time scales supported by the engine.
	*/
	enum class timeScale {ms, s};


	////////////
	//Classes.//
	////////////

	/**
	* @brief Used to represent time points in the engine.
	*/
	class time {

	public:

		// Observers.
		
		/**
		* @brief Get the actual system time point.
		*/
		template <timeScale scale>
		static double actualTime();

	private:

		// Constructors.

		time();

	};

	template <>
	inline static double time::actualTime<timeScale::ms>() {
	
		#if GRAPHICS_API == OPENGL

			return glfwGetTime() * 1000;

		#else

		#endif
	
	}

	template <>
	inline static double time::actualTime<timeScale::s>() {
	
		#if GRAPHICS_API == OPENGL

			return glfwGetTime();

		#else

		#endif
	
	}

	inline time::time() {}

}

#endif