#ifndef _GAMEWINDOWS_
#define _GAMEWINDOWS_
#include <GLFW/glfw3.h>

namespace VoxelEng
{

	class window 
	{
	
	public:

		window();

		GLFWwindow*& windowGLFW();

	private:

		GLFWwindow* GLFWwindow_;
	
	};

	inline GLFWwindow*& window::windowGLFW()
	{
	
		return GLFWwindow_;
	
	}

}

#endif