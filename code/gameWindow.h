/**
* @file gameWindow.h
* @version 1.0
* @date 20/04/2023
* @author Abdon Crespo Alvarez
* @title Game window.
* @brief Contains the definition of the class
* that abstracts the engine's windows used in its graphical part.
*/
#ifndef _VOXELENG_GAMEWINDOW_
#define _VOXELENG_GAMEWINDOW_

#include <string>
#include "definitions.h"

#if GRAPHICS_API == OPENGL

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#endif


namespace VoxelEng {

	/////////////////////////
	//Forward declarations.//
	/////////////////////////
	class camera;


	////////////
	//Classes.//
	////////////

	/**
	* @brief Abstraction of the engine's graphical part window.
	*/
	class window {

	public:

		// Constructors.

		/**
		* @brief Creates and shows up a new window.
		*/
		window(unsigned int width, unsigned int height, const std::string& name);


		// Observers.

		/**
		* @brief Returns the window's width.
		*/
		unsigned int width() const;

		/**
		* @brief Returns the window's height.
		*/
		unsigned int height() const;

		/**
		* @brief Returns true if the window is being closed or false otherwise.
		*/
		bool isClosing() const;

		/**
		* @brief Returns the window's width.
		*/
		const std::string& name() const;

		/**
		* @brief Returns true if the window has been resized or false otherwise.
		*/
		bool wasResized() const;

		/**
		* @brief Returns the window's aspect ratio.
		*/
		float aspectRatio() const;

		/**
		* @brief Returns true if the window is currently not locking the user's mouse cursor.
		*/
		bool isMouseFree() const;


		// Modifiers.

		/**
		* @brief Returns the window's pointer used by the current graphic API.
		*/
		GraphicsAPIWindow*& windowAPIpointer();

		/**
		* @brief Returns the user's camera that is assignated to this window.
		*/
		camera*& playerCamera();

		/**
		* @brief Returns the window's name.
		*/
		std::string& name();

		/**
		* @brief Used to set the window's flag that tells if it was resized or not.
		*/
		bool& wasResized();

		/**
		* @brief Set whether the window should lock the user's 
		* mouse cursor when it has focus or not.
		*/
		void changeStateMouseLock(bool isEnabled);

		/**
		* @brief Returns the window's aspect ratio.
		*/
		void changeStateMouseLock();

		/**
		* @brief Sets the new window's width and height to resize the window.
		* This functions is used for the GLFW window size callback. Because
		* of this, it gets executed each time the window's size IS MODIFIED
		* (not when the user stops resizing the window with the desired width and height).
		*/
		void resize(unsigned int width, unsigned int height);

		/**
		* @brief Windows' resizing heavy processing tasks are executed here.
		* This function should only be called when wasResized() equals to false
		* since that indicates that the user has stopped resizing the window.
		*/
		void resizeHeavyProcessing();

		/**
		* @brief Callback used for window resizing using OpenGL and GLFW.
		*/
		static void windowSizeCallback(GraphicsAPIWindow* window, int width, int height);
		
	private:

		/*
		Attributes.
		*/

		GraphicsAPIWindow* APIwindow_; // Graphic-API-specific window pointer.
		unsigned int width_,
					 height_;
		float aspectRatio_;
		double oldMouseX_, // Last cursor's X and Y coordinates before it was unlocked.
			   oldMouseY_;
		bool wasResized_,
			 isMouseFree_;
		std::string name_;
		camera* playerCamera_;

		
		/*
		Methods.
		*/

		void setMousePos(double x, double y);

		void getMousePos(double& x, double& y);
		
	};

	inline unsigned int window::width() const {

		return width_;

	}

	inline unsigned int window::height() const {

		return height_;

	}

	inline bool window::isClosing() const {
	
		return glfwWindowShouldClose(APIwindow_);
	
	}

	inline const std::string& window::name() const {

		return name_;

	}

	inline bool window::wasResized() const {
	
		return wasResized_;
	
	}

	inline float window::aspectRatio() const {
	
		return aspectRatio_;

	}

	inline GraphicsAPIWindow*& window::windowAPIpointer() {

		return APIwindow_;

	}

	inline camera*& window::playerCamera() {
	
		return playerCamera_;
	
	}

	inline std::string& window::name() {
	
		return name_;
	
	}

	inline bool& window::wasResized() {
	
		return wasResized_;
	
	}

	inline bool window::isMouseFree() const {
	
		return isMouseFree_;
	
	}

}

#endif