#include "gameWindow.h"
#include <stdexcept>
#include "gui.h"
#include "graphics.h"


namespace VoxelEng
{

	window::window(unsigned int width, unsigned int height, const std::string& name)
		: width_(width), height_(height), name_(name), playerCamera_(nullptr), wasResized_(false), isMouseFree_(false)
	{
	
		aspectRatio_ = (float)width / height;
	
	}

	void window::changeStateMouseLock(bool isEnabled)
	{
		
		isMouseFree_ = isEnabled;

		glfwSetInputMode(APIwindow_, GLFW_CURSOR, isMouseFree_ ? GLFW_CURSOR_NORMAL : GLFW_CURSOR_DISABLED);

		if (isMouseFree_)
			centerMouse();
		// restaurar antiguas coordenadas del ratón al volver a lockearlo para que no explote la camara
	
	}

	void window::changeStateMouseLock()
	{

		isMouseFree_ = !isMouseFree_;

		glfwSetInputMode(APIwindow_, GLFW_CURSOR, isMouseFree_ ? GLFW_CURSOR_NORMAL : GLFW_CURSOR_DISABLED);

		if (isMouseFree_)
			centerMouse();

	}

	void window::centerMouse() {
	
		glfwSetCursorPos(this->APIwindow_, width_/2.0f, height_ / 2.0f);
	
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

		graphics::getMainWindow()->resize(width, height);

	}

	void window::resizeHeavyProcessing() {
	
		width_ += width_ % 2 != 0;
		height_ += height_ % 2 != 0;

		glViewport(0, 0, width_, height_);

		playerCamera_->updateProjectionMatrix();

		wasResized_ = false;
	
	}

}