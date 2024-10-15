#ifndef _VOXELENG_LIGHTINSTANCE_
#define _VOXELENG_LIGHTINSTANCE_

#include <Registry/registryElement.h>
#include <vec.h>

namespace VoxelEng {

	/**
	* @brief Class used to represent an instance of a concrete
	* registered type of light in the world.
	*/
	class lightInstance : public registryElement {

	public:

		// TODO. OPTIMIZE THE SPACE OCUPPIED BY THIS STRUCTURE. ARE THE VEC4 REQUIRED OR CAN WE GO WITH ONLY VEC3?
		vec3 pos; // Unused in directional light. // Fourth value is for padding.
		float padding1;
		vec3 dir; // Fourth value is for padding.
		float padding2;
		unsigned int lightTypeIndex; // Inside the light type UBO, which light it is.

	};

}

#endif