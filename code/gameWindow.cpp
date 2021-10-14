#include "gameWindow.h"
#include <stdexcept>
#include "graphics.h"


namespace VoxelEng
{

	window::window(unsigned int width, unsigned int height, const std::string& name)
		: width_(width), height_(height), name_(name), playerCamera_(nullptr)
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

		glViewport(0, 0, width_, height_);

		playerCamera_->updateProjectionMatrix();

	}

	void window::windowSizeCallback(GraphicsAPIWindow* windowPointer, int width, int height)
	{

		graphics::getWindowCallbackPtr()->resize(width, height);

	}

}