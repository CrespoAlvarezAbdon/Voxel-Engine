/**
* @file controls.h
* @version 1.0
* @date 20/04/2023
* @author Abdon Crespo Alvarez
* @title Controls.
* @brief Contains the definition of the control keys supported by the engine
* and how to map said controls between the input APIs.
*/
#ifndef _VOXELENG_CONTROLS_
#define _VOXELENG_CONTROLS_

#include <unordered_map>
#include "definitions.h"

#if GRAPHICS_API == OPENGL

#include <GL/glew.h>
#include <GLFW/glfw3.h>


#endif


namespace VoxelEng {

	////////////////////
	//Classes & enums.//
	////////////////////

	/**
	* @brief Control key codes accepted and supported by the engine.
	*/
	enum class controlCode {

		noKey,
		alpha1,
		alpha2,
		alpha3,
		alpha4,
		alpha5,
		alpha6,
		alpha7,
		alpha8,
		alpha9,
		alpha0,
		q,
		w,
		e,
		r,
		t,
		y,
		u,
		i,
		o,
		p,
		a,
		s,
		d,
		f,
		g,
		h,
		j,
		k,
		l,
		ñ,
		z,
		x,
		c,
		v,
		b,
		n,
		m,
		comma,
		period,
		leftShift,
		rightShift,
		leftControl,
		rightControl,
		leftAlt,
		rightAlt,
		questionMarkUp,
		questionMarkDown,
		exclMarkUp,
		exckMarkDown,
		space,
		apostrophe,
		upArrow,
		downArrow,
		leftArrow,
		rightArrow,
		leftButton,
		rightButton,
		middleButton,
		escape

	};

	/**
	* @brief Provides mappings between the control codes supported by the game engine
	* and the control codes supported by the APIs that are currently used to get the
	* user input.
	*/
	class controls {

	public:

		// Observers.

		/**
		* @brief Returns the equivalent control code from VoxelEngine's API in GLFW.
		*/
		static unsigned int getGLFWControlCode(controlCode keyCode);

	private:

		static std::unordered_map<controlCode, unsigned int> GLFWcontrolCodes_;

	};

}

#endif 