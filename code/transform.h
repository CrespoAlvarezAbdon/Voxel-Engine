#ifndef _VOXELENG_TRANSFORM_
#define _VOXELENG_TRANSFORM_

#include "vec.h"

namespace VoxelEng {

	/**
	* @brief A transform represents the position, rotation and scale of an entity in the world.
	*/
	struct transform {

		transform()
		: position(vec3Zero), rotation(vec3Zero), scale{1.0f, 1.0f, 1.0f}
		{}

		vec3 position;
		vec3 rotation;
		vec3 scale;

	};

}

#endif