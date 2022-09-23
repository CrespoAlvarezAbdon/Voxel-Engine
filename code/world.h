#ifndef _VOXELENGWORLD_
#define _VOXELENGWORLD_
#include <unordered_set>
#include "chunk.h"
#include "graphics.h"
#include "definitions.h"


namespace VoxelEng {

	/////////////////////
	//Type definitions.//
	/////////////////////

	/*
	WARNING: W.I.P
	*/
	typedef color skybox;


	////////////
	//Classes.//
	////////////

	/*
	(W.I.P)
	Worlds are the greatest level of terraub organization in the engine.
	A world is defined by its biomes, gravity, global
	temperature and other physical and maybe magical properties.
	*/
	class world {
	
	public:

		// Modifiers.

		/*
		Red, green and blue channels must be in the range 0-255
		and alpha must be in the range 0-1.
		*/
		static void setSkybox(const skybox& skybox);

	private:


	
	};

	inline void world::setSkybox(const skybox& background) {

		#if GRAPHICS_API == OPENGL

			glClearColor(background.red / 255.0f, background.green / 255.0f, background.blue / 255.0f, background.alpha);

		#else

			

		#endif
	
	}

}

#endif