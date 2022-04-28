#include "controls.h"
#include <stdexcept>


namespace VoxelEng {

	// 'controls' class.

	std::unordered_map<key, unsigned int> controls::GLFWkeys_ = {
	
		{key::alpha1, GLFW_KEY_1},
		{key::alpha2, GLFW_KEY_2},
		{key::alpha3, GLFW_KEY_3},
		{key::alpha4, GLFW_KEY_4},
		{key::alpha5, GLFW_KEY_5},
		{key::alpha6, GLFW_KEY_6},
		{key::alpha7, GLFW_KEY_7},
		{key::alpha8, GLFW_KEY_8},
		{key::alpha9, GLFW_KEY_9},
		{key::alpha0, GLFW_KEY_0},
		{key::a, GLFW_KEY_A},
		{key::b, GLFW_KEY_B},
		{key::c, GLFW_KEY_C},
		{key::d, GLFW_KEY_D},
		{key::e, GLFW_KEY_E},
		{key::f, GLFW_KEY_F},
		{key::g, GLFW_KEY_G},
		{key::h, GLFW_KEY_H},
		{key::i, GLFW_KEY_I},
		{key::j, GLFW_KEY_J},
		{key::k, GLFW_KEY_K},
		{key::l, GLFW_KEY_L},
		{key::m, GLFW_KEY_M},
		{key::n, GLFW_KEY_N},
		{key::o, GLFW_KEY_O},
		{key::p, GLFW_KEY_P},
		{key::q, GLFW_KEY_Q},
		{key::r, GLFW_KEY_R},
		{key::s, GLFW_KEY_S},
		{key::t, GLFW_KEY_T},
		{key::u, GLFW_KEY_U},
		{key::v, GLFW_KEY_V},
		{key::w, GLFW_KEY_W},
		{key::x, GLFW_KEY_X},
		{key::y, GLFW_KEY_Y},
		{key::z, GLFW_KEY_Z},
		{key::comma, GLFW_KEY_COMMA},
		{key::period, GLFW_KEY_PERIOD},
		{key::leftShift, GLFW_KEY_LEFT_SHIFT},
		{key::rightShift, GLFW_KEY_RIGHT_SHIFT},
		{key::leftControl, GLFW_KEY_LEFT_CONTROL},
		{key::rightControl, GLFW_KEY_RIGHT_CONTROL},
		{key::leftAlt, GLFW_KEY_LEFT_ALT},
		{key::rightAlt, GLFW_KEY_RIGHT_ALT},
		{key::space, GLFW_KEY_SPACE},
		{key::apostrophe, GLFW_KEY_APOSTROPHE},
		{key::upArrow, GLFW_KEY_UP},
		{key::downArrow, GLFW_KEY_DOWN},
		{key::rightArrow, GLFW_KEY_RIGHT},
		{key::leftArrow, GLFW_KEY_LEFT},
	
	};

	unsigned int controls::getGLFWKey(key key) {
	
		if (GLFWkeys_.find(key) != GLFWkeys_.cend())
			return GLFWkeys_[key];
		else
			throw std::runtime_error("[ERROR]: Key is not registered in the input API currently used!");
	
	}

}
