#ifndef _VOXELENGWORLD_
#define _VOXELENGWORLD_

#include <unordered_set>
#include "chunk.h"
#include "graphics.h"


namespace VoxelEng
{

	///////////
	//Classes//
	///////////

	typedef color skybox; // W.I.P. Will have its own definition.

	/*
	Worlds are the greatest level of organization in the game.
	A world is defined by its biomes (W.I.P), gravity, global
	temperature and other physical and maybe magical properties.
	*/
	class world
	{
	
	public:

		// Constructors

		/*
		WARNING. Two worlds cannot share the same ID!
		*/
		world(unsigned int id);


		// Modifiers.

		/*
		Red, green and blue channels must be in the range 0-255
		and alpha must be in the range 0-1.
		*/
		void setBackground(const skybox& skybox);


		// Other methods.

		void loadChunk(const chunkPos& pos);

		void unloadChunk(const chunkPos& pos);

	private:

		unsigned int id_;
		static std::unordered_set<unsigned int> worlds_;
	
	};

	inline void world::setBackground(const skybox& background)
	{

		glClearColor(background.red / 255.0f, background.green / 255.0f, background.blue / 255.0f, background.alpha);
	
	}

}

#endif