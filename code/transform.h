#ifndef _VOXELENG_TRANSFORM_
#define _VOXELENG_TRANSFORM_

#include "vec.h"

namespace VoxelEng {

	/**
	* @brief A transform represents the position, rotation and scale of an entity in the world.
	*/
	struct transform {

		transform()
		: position(vec3Zero), chunkPosition(vec3Zero), rotation(vec3Zero), scale{1.0f, 1.0f, 1.0f},
		  Xaxis(vec3FixedNorth), Yaxis(vec3FixedUp), Zaxis(vec3FixedEast),
		  viewDirection(vec3FixedNorth),
		  gravityDirection(vec3FixedDown)
		{}

		vec3 position,
			 chunkPosition,
			 rotation,
			 scale,
			 Xaxis,
			 Yaxis,
			 Zaxis,
			 viewDirection,
			 gravityDirection;

	};

}

#endif