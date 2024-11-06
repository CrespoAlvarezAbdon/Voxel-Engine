#ifndef _VOXELENG_LIGHTINSTANCE_
#define _VOXELENG_LIGHTINSTANCE_

#include <Registry/registryElement.h>
#include <vec.h>

#if GRAPHICS_API == OPENGL

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm.hpp>

#endif

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
		vec3 dir;
		float lightTypeIndex;
		glm::mat4 MVP;

	};

}

#endif