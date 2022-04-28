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

		bool wasResized() const;

		float aspectRatio() const;


		// Modifiers.

		/*
		Returns the window's pointer used by the current graphic API.
		*/
		GraphicsAPIWindow*& windowAPIpointer();

		camera*& playerCamera();

		std::string& name();

		bool& wasResized();


		// Other methods.

		void lockMouse();

		void unlockMouse();

		/*
		Gets the new window's width and height to resize the window.
		This functions is used for the GLFW window size callback. Because
		of this, it gets executed each time the window's size IS MODIFIED
		(not when the user stops resizing the window with the desired width and height).
		*/
		void resize(unsigned int width, unsigned int height);

		/*
		Windows' resizing heavy processing tasks are executed here.
		This function should only be called when wasResized() equals to false
		since that indicates that the user has stopped resizing the window.
		*/
		void resizeHeavyProcessing();

		/*
		Callback used for window resizing using OpenGL and GLFW.
		*/
		static void windowSizeCallback(GraphicsAPIWindow* window, int width, int height);
		
	private:

		GraphicsAPIWindow* APIwindow_; // Graphic-API-specific window pointer.
		unsigned int width_,
					 height_;
		float aspectRatio_;
		bool wasResized_;
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

	inline bool window::wasResized() const {
	
		return wasResized_;
	
	}

	inline float window::aspectRatio() const {
	
		return aspectRatio_;

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

	inline bool& window::wasResized() {
	
		return wasResized_;
	
	}

}

#endif