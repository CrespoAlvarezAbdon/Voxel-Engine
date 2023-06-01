#include "gameWindow.h"
#include <stdexcept>
#include "gui.h"
#include "graphics.h"
#include "definitions.h"


namespace VoxelEng {

	window::window(unsigned int width, unsigned int height, const std::string& name)
		: width_(width), height_(height), name_(name), playerCamera_(nullptr), wasResized_(false), isMouseFree_(false) {
	
		aspectRatio_ = (float)width / height;
	
	}

	void window::changeStateMouseLock(bool isEnabled) {

		if (!isMouseFree_ && isEnabled)
			getMousePos(oldMouseX_, oldMouseY_);
		else if (isMouseFree_ && !isEnabled)
			setMousePos(oldMouseX_, oldMouseY_);

		isMouseFree_ = isEnabled;

		#if GRAPHICS_API == OPENGL

			glfwSetInputMode(APIwindow_, GLFW_CURSOR, isMouseFree_ ? GLFW_CURSOR_NORMAL : GLFW_CURSOR_DISABLED);

		#else



		#endif

		if (isMouseFree_)
			setMousePos(width_ / 2.0f, height_ / 2.0f);
	
	}

	void window::changeStateMouseLock() {

		if (!isMouseFree_)
			getMousePos(oldMouseX_, oldMouseY_);
		else
			setMousePos(oldMouseX_, oldMouseY_);

		isMouseFree_ = !isMouseFree_;

		#if GRAPHICS_API == OPENGL

			glfwSetInputMode(APIwindow_, GLFW_CURSOR, isMouseFree_ ? GLFW_CURSOR_NORMAL : GLFW_CURSOR_DISABLED);

		#else



		#endif

		if (isMouseFree_)
			setMousePos(width_ / 2.0f, height_ / 2.0f);

	}

	void window::setMousePos(double x, double y) {
	
		#if GRAPHICS_API == OPENGL

			glfwSetCursorPos(this->APIwindow_, x, y);

		#else



		#endif

	}

	void window::getMousePos(double& x, double& y) {
	
		#if GRAPHICS_API == OPENGL

			glfwGetCursorPos(this->APIwindow_, &x, &y);

		#else



		#endif

	}

	void window::resize(unsigned int width, unsigned int height) {

		if (width != 0 && height != 0) {
		
			width_ = width;
			height_ = height;

			aspectRatio_ = (float)width_ / height_;

			wasResized_ = true;
		
		}

	}

	void window::windowSizeCallback(GraphicsAPIWindow* windowPointer, int width, int height) {

		graphics::getMainWindow()->resize(width, height);

	}

	void window::resizeHeavyProcessing() {

		#if GRAPHICS_API == OPENGL

			glViewport(0, 0, width_, height_);

		#else



		#endif

		playerCamera_->updateProjectionMatrix();

		GUImanager::updateOrthoMatrix();

		wasResized_ = false;
	
	}

}