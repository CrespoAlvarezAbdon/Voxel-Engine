#ifndef _VOXELENG_BLOCK_LIGHT_MODIFICATION_
#define _VOXELENG_BLOCK_LIGHT_MODIFICATION_

#include <type_traits>

#include <vec.h>
#include <Chunk/blockLight.hpp>

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
		
		/*
		Methods.
		*/

		// Constructors

		/**
		* @brief Default constructor
		*/
		blockLightMod();

		/**
		* @brief Class constructor
		* @param pos Block light modification position
		* @param color Block light modification color
		*/
		blockLightMod(const ivec3& pos, const blockLight& color);


		/*
		Attributes
		*/

		/**
		* @brief Position the light is being propagated to.
		*/
		ivec3 pos;

		/**
		* Light's color applied to its position. Last value is alpha.
		*/
		blockLight color;

	};

	inline blockLightMod::blockLightMod() 
	: pos(vec3Zero) {}

	inline blockLightMod::blockLightMod(const ivec3& pos, const blockLight& color)
	: pos(pos), color(color) {}

}

namespace std {
	
	template<>
	struct hash<VoxelEng::blockLightMod> {

		size_t operator()(const VoxelEng::blockLightMod& t) const {
		
			glm::ivec3 v;
			v.x = t.pos.x;
			v.y = t.pos.y;
			v.z = t.pos.z;
			hash<glm::ivec3> h;
			return h(v);

		}

	};

}

#endif