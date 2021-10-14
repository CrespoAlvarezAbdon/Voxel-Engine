#ifndef _GAMEWINDOWS_
#define _GAMEWINDOWS_
#include <string>
#include "definitions.h"
#include "camera.h"
#include <GLFW/glfw3.h>


//////////////////////////////
//Forward class declarations//
//////////////////////////////
class camera;

namespace VoxelEng
{

	///////////
	//Classes//
	///////////

	/*
	Abstraction of a window.
	*/
	class window 
	{

	public:

		// Constructors.

		/*
		Creates and shows up a new window.
		*/
		window(unsigned int width, unsigned int height, const std::string& name);


		// Observers.

		unsigned int width() const;

		unsigned int height() const;

		bool isClosing() const;

		const std::string& name() const;


		// Modifiers.

		/*
		Returns the window's pointer used by the current graphic API.
		*/
		GraphicsAPIWindow*& windowAPIpointer();

		camera*& playerCamera();

		std::string& name();


		// Other methods.

		void lockMouse();

		void unlockMouse();

		void resize(unsigned int width, unsigned int height);

		/*
		Callback used for window resizing using OpenGL and GLFW.
		*/
		static void windowSizeCallback(GraphicsAPIWindow* window, int width, int height);

	private:

		GraphicsAPIWindow* APIwindow_; // Graphic-API-specific window pointer.
		unsigned int width_,
					 height_;
		std::string name_;
		camera* playerCamera_;

	};

	inline unsigned int window::width() const
	{

		return width_;

	}

	inline unsigned int window::height() const
	{

		return height_;

	}

	inline bool window::isClosing() const
	{
	
		return glfwWindowShouldClose(APIwindow_);
	
	}

	inline const std::string& window::name() const
	{

		return name_;

	}

	inline GraphicsAPIWindow*& window::windowAPIpointer()
	{

		return APIwindow_;

	}

	inline camera*&  window::playerCamera()
	{
	
		return playerCamera_;
	
	}

	inline std::string& window::name()
	{
	
		return name_;
	
	}

}

#endif