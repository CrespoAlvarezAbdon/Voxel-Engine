#include "gameWindow.h"
#include <stdexcept>
#include "gui.h"
#include "graphics.h"


namespace VoxelEng
{

	window::window(unsigned int width, unsigned int height, const std::string& name)
		: width_(width), height_(height), name_(name), playerCamera_(nullptr), wasResized_(false)
	{
	
		aspectRatio_ = (float)width / height;
	
	}

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

		aspectRatio_ = (float)width_ / height_;

		wasResized_ = true;

	}

	void window::windowSizeCallback(GraphicsAPIWindow* windowPointer, int width, int height)
	{

		graphics::getWindowCallbackPtr()->resize(width, height);

	}

	void window::resizeHeavyProcessing() {
	
		width_ += width_ % 2 != 0;
		height_ += height_ % 2 != 0;

		glViewport(0, 0, width_, height_);

		playerCamera_->updateProjectionMatrix();

		wasResized_ = false;
	
	}

}