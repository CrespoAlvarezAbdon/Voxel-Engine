#include "graphics.h"
#include <stdexcept>
#include "gameWindow.h"
#include "errors.hpp"
#include <GLFW/glfw3.h>

namespace VoxelEng
{

	// 'color' struct.

	color::color(float r, float g, float b, float a)
	{
	
		if (r < 0.0f || g < 0.0f || b < 0.0f || a < 0.0f)
			throw new std::runtime_error("[ERROR]: Atleast one of the channels defined in a 'VoxelEng::color' object is negative!");
		else
		{

			red = r;
			green = g;
			blue = b;
			alpha = a;

		}
	
	}

	// 'graphics' class.

	void graphics::setVSync(bool isEnabled)
	{

		glfwSwapInterval(isEnabled);

	}

	void graphics::setDepthTest(bool isEnabled)
	{

		if (isEnabled)
			glEnable(GL_DEPTH_TEST);
		else
			glDisable(GL_DEPTH_TEST);

	}

	void graphics::setBasicFaceCulling(bool isEnabled)
	{

		if (isEnabled)
			glEnable(GL_CULL_FACE);
		else
			glDisable(GL_CULL_FACE);

	}

	void graphics::setTransparency(bool isEnabled)
	{
	
		if (isEnabled)
		{
		
			glEnable(GL_BLEND);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		
		}
		else
			glDisable(GL_BLEND);
		
	}

	void graphics::initialize(window& renderingWindow)
	{
	
		// GLFW initialization.
		if (!glfwInit())
			throw new std::runtime_error("[ERROR]: Failed to initialize the GLFW library!");

		// Select GLFW version.
		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
		glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

		// Create API rendering window.
		renderingWindow.windowAPIpointer() = glfwCreateWindow((int)renderingWindow.width(), (int)renderingWindow.height(), renderingWindow.name().data(), NULL, NULL);

		if (!renderingWindow.windowAPIpointer())
		{

			glfwTerminate();
			throw std::runtime_error("[ERROR]: Failed to create window named " + renderingWindow.name());

		}


		glfwMakeContextCurrent(renderingWindow.windowAPIpointer());

		// With the previously created context, now we can initialize the GLEW library.
		if (glewInit() != GLEW_OK)
			throw error(error::errorTypes::GLEW_INIT_FAILED);
	
	}

	window* graphics::windowCallbackPtr_ = nullptr;
	player* graphics::playerCallbackPtr_ = nullptr;

}