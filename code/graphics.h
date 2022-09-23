#ifndef _VOXELENG_GRAPHICS_
#define _VOXELENG_GRAPHICS_
#include "gameWindow.h"


namespace VoxelEng {	

	////////////
	//Structs.//
	////////////

	struct color {
	
		color(float red, float green, float blue, float alpha);

		float red,
			  green,
			  blue,
			  alpha;
	
	};


	////////////
	//Classes.//
	////////////

	/*
	Class used to abstract the graphics API's calls independently
	of the underlying graphics API (directX, OpenGL...) that is
	being used.
	*/
	class graphics {
	
	public:

		// Initialisers.

		/*
		Initializes the underlying graphics APIs/libraries.
		WARNING. Must be called in a thread with valid graphic API context.
		*/
		static void init(window& renderingWindow);


		// Observers.

		/*
		Gets the pointer to the 'VoxelEng::window' class object that is being used
		to handle the game window's callbacks.
		WARNING. Must be called in a thread with valid graphic API context.
		*/
		static window* getMainWindow();

		static bool initialised();


		// Modifiers.

		/*
		Sets the pointer main window in the engine's graphical mode.
		The main window pointer is used to handle the game's main window callbacks.
		WARNING. Must be called in a thread with valid graphic API context.
		*/
		static void setMainWindow(window* windowCallbackPtr);

		/*
		Set Vertical Synchronization (VSync) on (true) or off (false).
		WARNING. Must be called in a thread with valid graphic API context.
		*/
		static void setVSync(bool isEnabled);

		/*
		Set Depth test on (true) or off (false).
		WARNING. Must be called in a thread with valid graphic API context.
		*/
		static void setDepthTest(bool isEnabled);

		/*
		Set basic face culling on (true) or off (false).
		WARNING. Must be called in a thread with valid graphic API context.
		*/
		static void setBasicFaceCulling(bool isEnabled);

		/*
		Set transparency on (true) or off (false).
		WARNING. Must be called in a thread with valid graphic API context.
		*/
		static void setTransparency(bool isEnabled);


		// Clean up.

		static void cleanUp();

	private:

		static bool initialised_;
		static window* mainWindow_;
		
	};

	inline window* graphics::getMainWindow() {
	
		return mainWindow_;
	
	}

	inline bool graphics::initialised() {
	
		return initialised_;
	
	}

	inline void graphics::setMainWindow(window* windowPointer) {

		mainWindow_ = windowPointer;

	}

}

#endif