#ifndef _VOXELENG_CONTROLS_
#define _VOXELENG_CONTROLS_
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <unordered_map>


namespace VoxelEng {

	////////////////////
	//Classes & enums.//
	////////////////////

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
		middleButton

	};


	class controls {

	public:

		// Observers.

		/*
		Returns the equivalent control code from VoxelEngine's API in GLFW.
		*/
		static unsigned int getGLFWControlCode(controlCode keyCode);

	private:

		static std::unordered_map<controlCode, unsigned int> GLFWcontrolCodes_;

	};

}

#endif 