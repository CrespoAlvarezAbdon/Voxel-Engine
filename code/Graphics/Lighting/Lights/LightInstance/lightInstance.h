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

		/**
		* @brief Class constructor.
		* @param pos Light's position in global-grid-coordinates.
		* @param dir Light's direction (useful only for directional lights and spotlights at the moment).
		* @param MVP Light's ModelViewProjection Matrix.
		*/
		lightInstance(const vec3& pos = vec3Zero, const vec3& dir = vec3Zero, float lightTypeIndex = 0, const glm::mat4& MVP = {});

		// TODO. OPTIMIZE THE SPACE OCUPPIED BY THIS STRUCTURE. ARE THE VEC4 REQUIRED OR CAN WE GO WITH ONLY VEC3?
		vec3 pos; // Unused in directional light.
		float padding1; // For padding due to GPU's memory constraints.
		vec3 dir;
		float lightTypeIndex;
		glm::mat4 MVP;

	};

	inline lightInstance::lightInstance(const vec3& pos, const vec3& dir, float lightTypeIndex, const glm::mat4& MVP)
	: pos(pos), padding1(0), dir(dir), lightTypeIndex(lightTypeIndex), MVP(MVP){}
}

#endif