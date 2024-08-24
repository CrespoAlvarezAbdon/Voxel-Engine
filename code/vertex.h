/**
* @file vertex.h
* @version 1.0
* @date 25/04/2023
* @author Abdon Crespo Alvarez
* @title Vertex.
* @brief Contains the definition of the 3D and 2D vertices used by the engine.
*/
#ifndef _VOXELENG_VERTEX_
#define _VOXELENG_VERTEX_

#include "definitions.h"

#if GRAPHICS_API == OPENGL

#include <GL/glew.h>

#endif


namespace VoxelEng {

	///////////////////
	//Classes/Structs//
	///////////////////


	/**
	* @brief Represents a 3D vertex with texture coordinates.
	*/
	struct vertex {

		vertexCoord positions[3] = {0,0,0}; // 0 = coord in X axis, 1 = coord in Y axis and 2 = coord in Z axis.
		textureCoord textureCoords[2] = {0,0};
		// The member 'normals' follows GL_INT_2_10_10_10_REV format. That is, 10 first bits are assigned for first normal coord, 
		// the 10 next for the second normal coord... and the last 2 bits are unused (for now) because of alignment reasons.
		unsigned char color[4] = {255,255,255,255}; // RGBA stored in 32-bits.
		unsigned char additionalData[4] = { 0,0,0,0 }; // First byte is material index.
		normalVec normals = 0;

	};


	/**
	* @brief Represents a 2D vertex (usually used for drawing GUI).
	*/
	struct vertex2D {

		vertexCoord positions[2];
		textureCoord textureCoords[2];

	};

}

#endif