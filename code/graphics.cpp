#include "graphics.h"

#if GRAPHICS_API == OPENGL

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#endif

#include "logger.h"
#include "definitions.h"


namespace VoxelEng {

	// 'color' struct.

	color::color(float r, float g, float b, float a) {
	
		if (r < 0.0f || g < 0.0f || b < 0.0f || a < 0.0f)
			logger::errorLog("Atleast one of the channels defined in a 'VoxelEng::color' object is negative");
		else {

			red_ = r;
			green_ = g;
			blue_ = b;
			alpha_ = a;

		}
	
	}

	// 'graphics' class.

	bool graphics::initialised_ = false;
	window* graphics::mainWindow_ = nullptr;


	void graphics::init(window& mainWindow) {

		if (initialised_)
			logger::errorLog("Graphics API is already initialised");
		else {

			#if GRAPHICS_API == OPENGL

				// GLFW initialization.
				if (!glfwInit())
					logger::errorLog("Failed to initialize the GLFW library!");

				// Select GLFW version.
				glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
				glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
				glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

				// Create API rendering window.
				mainWindow.windowAPIpointer() = glfwCreateWindow((int)mainWindow.width(), (int)mainWindow.height(), mainWindow.name().data(), NULL, NULL);

				if (!mainWindow.windowAPIpointer()) {

					glfwTerminate();
					logger::errorLog("Failed to create window named " + mainWindow.name());

				}

				mainWindow_ = &mainWindow;
				glfwMakeContextCurrent(mainWindow.windowAPIpointer());

				// With the previously created context, now we can initialize the GLEW library.
				if (glewInit() != GLEW_OK)
					logger::errorLog("GLEW_INIT_FAILED");

				graphics::setVSync(true);
				graphics::setDepthTest(true);
				graphics::setBasicFaceCulling(true);
				graphics::setTransparency(true);

			#else

				

			#endif

			initialised_ = true;
		
		}

	}

	void graphics::GLCheckErrors(std::ostream& os, const char* file, const char* function, unsigned int line) {

		bool errorsDetected = false;

		os << "[OpenGL error checking]" << std::endl;
		while (GLenum error = glGetError()) {

			errorsDetected = true;

			os << "Error: " << error << std::endl
				<< "at function " << function << ", instruction before line " << line << std::endl
				<< "in file " << file << std::endl << std::endl;

		}

		if (errorsDetected)
			abort();

	}

	void graphics::setDepthTest(bool isEnabled) {

		if (isEnabled)
			glEnable(GL_DEPTH_TEST);
		else
			glDisable(GL_DEPTH_TEST);

	}

	void graphics::setBasicFaceCulling(bool isEnabled) {

		if (isEnabled)
			glEnable(GL_CULL_FACE);
		else
			glDisable(GL_CULL_FACE);

	}

	void graphics::setTransparency(bool isEnabled) {
	
		if (isEnabled) {
		
			glEnable(GL_BLEND);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		
		}
		else
			glDisable(GL_BLEND);
		
	}
	
	void graphics::cleanUp() {
	
		#if GRAPHICS_API == OPENGL

            glfwTerminate();

        #else



        #endif

		initialised_ = false;
	
	}

}