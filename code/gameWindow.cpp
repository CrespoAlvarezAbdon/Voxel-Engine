#include "gameWindow.h"
#include <stdexcept>
#include "gui.h"
#include "graphics.h"

#include <iostream>
#include <ostream>


namespace VoxelEng
{

	window::window(unsigned int width, unsigned int height, const std::string& name)
		: width_(width), height_(height), name_(name), playerCamera_(nullptr), wasResized_(false)
	{}

	void window::lockMouse()
	{
		
		glfwSetInputMode(APIwindow_, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	
	}

	void window::unlockMouse()
	{
	
		glfwSetInputMode(APIwindow_, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
	
	}

	void window::resize(unsigned int width, unsigned int height)
	{

		width_ = width;
		height_ = height;

		wasResized_ = true;

	}

	void window::windowSizeCallback(GraphicsAPIWindow* windowPointer, int width, int height)
	{

		graphics::getWindowCallbackPtr()->resize(width, height);

	}

}