#include "controls.h"
#include "logger.h"


namespace VoxelEng {

	// 'controls' class.

	std::unordered_map<controlCode, unsigned int> controls::GLFWcontrolCodes_ = {
	
		{controlCode::alpha1, GLFW_KEY_1},
		{controlCode::alpha2, GLFW_KEY_2},
		{controlCode::alpha3, GLFW_KEY_3},
		{controlCode::alpha4, GLFW_KEY_4},
		{controlCode::alpha5, GLFW_KEY_5},
		{controlCode::alpha6, GLFW_KEY_6},
		{controlCode::alpha7, GLFW_KEY_7},
		{controlCode::alpha8, GLFW_KEY_8},
		{controlCode::alpha9, GLFW_KEY_9},
		{controlCode::alpha0, GLFW_KEY_0},
		{controlCode::a, GLFW_KEY_A},
		{controlCode::b, GLFW_KEY_B},
		{controlCode::c, GLFW_KEY_C},
		{controlCode::d, GLFW_KEY_D},
		{controlCode::e, GLFW_KEY_E},
		{controlCode::f, GLFW_KEY_F},
		{controlCode::g, GLFW_KEY_G},
		{controlCode::h, GLFW_KEY_H},
		{controlCode::i, GLFW_KEY_I},
		{controlCode::j, GLFW_KEY_J},
		{controlCode::k, GLFW_KEY_K},
		{controlCode::l, GLFW_KEY_L},
		{controlCode::m, GLFW_KEY_M},
		{controlCode::n, GLFW_KEY_N},
		{controlCode::o, GLFW_KEY_O},
		{controlCode::p, GLFW_KEY_P},
		{controlCode::q, GLFW_KEY_Q},
		{controlCode::r, GLFW_KEY_R},
		{controlCode::s, GLFW_KEY_S},
		{controlCode::t, GLFW_KEY_T},
		{controlCode::u, GLFW_KEY_U},
		{controlCode::v, GLFW_KEY_V},
		{controlCode::w, GLFW_KEY_W},
		{controlCode::x, GLFW_KEY_X},
		{controlCode::y, GLFW_KEY_Y},
		{controlCode::z, GLFW_KEY_Z},
		{controlCode::comma, GLFW_KEY_COMMA},
		{controlCode::period, GLFW_KEY_PERIOD},
		{controlCode::leftShift, GLFW_KEY_LEFT_SHIFT},
		{controlCode::rightShift, GLFW_KEY_RIGHT_SHIFT},
		{controlCode::leftControl, GLFW_KEY_LEFT_CONTROL},
		{controlCode::rightControl, GLFW_KEY_RIGHT_CONTROL},
		{controlCode::leftAlt, GLFW_KEY_LEFT_ALT},
		{controlCode::rightAlt, GLFW_KEY_RIGHT_ALT},
		{controlCode::space, GLFW_KEY_SPACE},
		{controlCode::apostrophe, GLFW_KEY_APOSTROPHE},
		{controlCode::upArrow, GLFW_KEY_UP},
		{controlCode::downArrow, GLFW_KEY_DOWN},
		{controlCode::rightArrow, GLFW_KEY_RIGHT},
		{controlCode::leftArrow, GLFW_KEY_LEFT},
		{controlCode::leftButton, GLFW_MOUSE_BUTTON_LEFT},
		{controlCode::rightButton, GLFW_MOUSE_BUTTON_RIGHT},
		{controlCode::leftButton, GLFW_MOUSE_BUTTON_MIDDLE},
	
	};

	unsigned int controls::getGLFWControlCode(controlCode controlCode) {
	
		if (GLFWcontrolCodes_.find(controlCode) != GLFWcontrolCodes_.cend())
			return GLFWcontrolCodes_[controlCode];
		else
			logger::errorLog("Control code is not registered in the input API currently used!");
	
	}

}
