#ifndef _GRAPHICS_
#define _GRAPHICS_

////////////////////////
//Forward declarations//
////////////////////////

class player;


namespace VoxelEng
{

	////////////////////////
	//Forward declarations//
	////////////////////////

	class window;

	///////////
	//Structs//
	///////////

	struct color
	{
	
		color(float red, float green, float blue, float alpha);

		float red,
			  green,
			  blue,
			  alpha;
	
	};


	///////////
	//Classes//
	///////////

	/*
	Class used to abstract the graphics API's calls independently
	of the underlying graphics API (directX, OpenGL...) that is
	being used.
	*/
	class graphics
	{
	
	public:

		// Observers.

		/*
		Gets the pointer to the 'VoxelEng::window' class object that is being used
		to handle the game window's callbacks.
		WARNING. Must be called in a thread with valid graphic API context.
		*/
		static window * getMainWindow();

		/*
		Gets the pointer to the 'VoxelEng::player' class object that is being used
		to handle the player input's callbacks.
		WARNING. Must be called in a thread with valid graphic API context.
		*/
		static player * getPlayerCallbackPtr();


		// Modifiers.

		/*
		Sets the pointer to the 'VoxelEng::window' class object that is being used
		to handle the game window's callbacks.
		WARNING. Must be called in a thread with valid graphic API context.
		*/
		static void setWindowCallbackPtr(window* windowCallbackPtr);

		/*
		Sets the pointer to the 'VoxelEng::player' class object that is being used
		to handle the player input's callbacks.
		WARNING. Must be called in a thread with valid graphic API context.
		*/
		static void setPlayerCallbackPtr(player* playerCallbackPtr);

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


		// Other methods.

		/*
		Initializes the underlying graphics APIs/libraries.
		WARNING. Must be called in a thread with valid graphic API context.
		*/
		static void initialize(window& renderingWindow);

	private:

		static window* windowCallbackPtr_;
		static player* playerCallbackPtr_;
		
	};

	inline window * graphics::getMainWindow()
	{
	
		return windowCallbackPtr_;
	
	}

	inline player * graphics::getPlayerCallbackPtr()
	{

		return playerCallbackPtr_;

	}

	inline void graphics::setWindowCallbackPtr(window* windowCallbackPtr)
	{

		windowCallbackPtr_ = windowCallbackPtr;

	}

	inline void graphics::setPlayerCallbackPtr(player* playerCallbackPtr)
	{

		playerCallbackPtr_ = playerCallbackPtr;

	}

}

#endif