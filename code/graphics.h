/**
* @file graphics.h
* @version 1.0
* @date 25/04/2023
* @author Abdon Crespo Alvarez
* @title Engine Graphics API.
* @brief Contains the engine's graphics API, which is in charge
* of abstracting the details of the actual graphics API used
* by the engine among other auxiliar data structures.
*/
#ifndef _VOXELENG_GRAPHICS_
#define _VOXELENG_GRAPHICS_
#include "gameWindow.h"


namespace VoxelEng {	

	////////////
	//Classes.//
	////////////

	/**
	* @brief Represents the diffent color formats supported by the engine.
	* Only RGBA colour supported for now.
	*/
	class color {
	
	public:

		// Constructors.

		/**
		* @brief Class constructor.
		*/
		color(float red, float green, float blue, float alpha);


		// Observers.

		/**
		* @brief Returns the color's red component.
		*/
		float red() const;

		/**
		* @brief Returns the color's green component.
		*/
		float green() const;

		/**
		* @brief Returns the color's blue component.
		*/
		float blue() const;

		/**
		* @brief Returns the color's alpha component.
		*/
		float alpha() const;


		// Modifiers.

		/**
		* @brief Sets the color's red component.
		*/
		float& red();

		/**
		* @brief Sets the color's green component.
		*/
		float& green();

		/**
		* @brief Sets the color's blue component.
		*/
		float& blue();

		/**
		* @brief Sets the color's alpha component.
		*/
		float& alpha();

	private:

		float red_,
			  green_,
			  blue_,
			  alpha_;
	
	};

	inline float color::red() const {
	
		return red_;
	
	}

	inline float color::green() const {
	
		return green_;
	
	}

	inline float color::blue() const {
	
		return blue_;
	
	}

	inline float color::alpha() const {
	
		return alpha_;
	
	}

	inline float& color::red() {

		return red_;

	}

	inline float& color::green() {

		return green_;

	}

	inline float& color::blue() {

		return blue_;

	}

	inline float& color::alpha() {

		return alpha_;

	}


	/**
	* @brief Class used to abstract the graphics operations independently
	* of the underlying graphics API (directX, OpenGL...) that is
	* being used.
	*/
	class graphics {
	
	public:

		// Initialisers.

		/**
		* @brief Initializes the underlying graphics APIs/libraries that
		* are used by the engine.
		*/
		static void init(window& renderingWindow);


		// Observers.

		/**
		* @brief Gets the pointer to the 'VoxelEng::window' class object that is being used
		* to handle the game window's callbacks.
		* WARNING. Must be called in a thread with valid graphic API context.
		*/
		static window* getMainWindow();

		/**
		* @brief Returns true if the engine graphics API is initialised or false otherwise.
		*/
		static bool initialised();

		/**
		* @brief Clear all previous thrown graphics API errors prior to this function's call.
		* WARNING. Must be called in a thread with valid graphics API context.
		*/
		static void eraseErrors();

		/**
		* @brief Obtain the graphics API's last errors.
		* The last three parameters should be used to help locate the error when debugging.
		* WARNING. Must be called in a thread with valid graphics API context.
		*/
		static void GLCheckErrors(std::ostream& os, const char* file, const char* function, unsigned int line);


		// Modifiers.

		/**
		* @brief Set Vertical Synchronization (VSync) on (true) or off (false).
		* WARNING. Must be called in a thread with valid graphics API context.
		*/
		static void setVSync(bool isEnabled);

		/**
		* @brief Set Depth test on (true) or off (false).
		* WARNING. Must be called in a thread with valid graphics API context.
		*/
		static void setDepthTest(bool isEnabled);

		/**
		* @brief Set basic face culling on (true) or off (false).
		* WARNING. Must be called in a thread with valid graphics API context.
		*/
		static void setBasicFaceCulling(bool isEnabled);

		/**
		* @brief Set transparency on (true) or off (false).
		* WARNING. Must be called in a thread with valid graphics API context.
		*/
		static void setTransparency(bool isEnabled);


		// Clean up.

		/**
		* @brief Deallocate any heap memory used by the engine graphics API and shut it down.
		*/
		static void cleanUp();

	private:

		static bool initialised_;
		static window* mainWindow_;
		
	};

	inline void graphics::eraseErrors() {

		while (glGetError());

	}

	inline window* graphics::getMainWindow() {
	
		return mainWindow_;
	
	}

	inline bool graphics::initialised() {
	
		return initialised_;
	
	}

	inline void graphics::setVSync(bool isEnabled) {

		glfwSwapInterval(isEnabled);

	}

}

#endif