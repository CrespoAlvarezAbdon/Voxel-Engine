#ifndef _VOXELENG_BLOCK_LIGHT_MODIFICATION_
#define _VOXELENG_BLOCK_LIGHT_MODIFICATION_

#include <type_traits>
#include <vec.h>

#if GRAPHICS_API == OPENGL

#include <GL/glew.h>
#include <glm.hpp>
#include <hash.hpp>

#endif

namespace VoxelEng {

	////////////
	//Structs.//
	////////////

	/**
	* @brief Definition of an instance of a block light that propagates through
	* the world using a flood-fill algorithm. This only represents the propagation of a certain
	* light source.
	*/
	struct blockLightMod {

		/**
		* @brief Position the light is being propagated to.
		*/
		basicVec3 pos;

		/**
		* @brief It determines whether the light's color is applied fully in its position or not.
		*/
		char intensity;

		/**
		* Light's color applied to its position. Last value is alpha.
		*/
		basicVec4 color;

	};

}

namespace std {
	
	template<>
	struct hash<VoxelEng::blockLightMod> {

		size_t operator()(const VoxelEng::blockLightMod& t) const {
		
			glm::vec3 v;
			v.x = t.pos.x;
			v.y = t.pos.y;
			v.z = t.pos.z;
			hash<glm::vec3> h;
			return h(v);

		}

	};

}

#endif