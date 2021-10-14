#include "world.h"
#include <stdexcept>
#include <string>


namespace VoxelEng
{

	std::unordered_set<unsigned int> world::worlds_;

	world::world(unsigned int id)
	{
	
		// Verify that there aren't two worlds with the same ID.
		if (!worlds_.contains(id))
		{

			id_ = id;
			worlds_.insert(id);

		}
		else
			throw std::runtime_error(std::string("[ERROR]: A world already exists with ID %d", id));
	
	}

	void world::loadChunk(const chunkPos& pos)
	{
	
		// W.I.P
	
	}

	void world::unloadChunk(const chunkPos& pos)
	{

		// W.I.P

	}

}