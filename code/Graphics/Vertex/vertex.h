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

#include <definitions.h>
#include <vec.h>

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

		// TOMORROW. REORGANIZE THIS.
		float positions[3] = {0,0,0}; // 0 = coord in X axis, 1 = coord in Y axis and 2 = coord in Z axis.
		float textureCoords[2] = {0,0};
		unsigned char color[4] = {255,255,255,255}; // RGBA stored in 32-bits.
		basic_vec4 additionalData; // Color light values.
		basic_vec4 lightExtraData; // First two bytes are barycentric coordinates of the vertex. Third byte is material index.
		basic_vec4 colorExtraData; // The colors of the four vertices of the same face block added like this: -ColorVertexA + ColorVertexB - ColorVertexC + ColorVertexD 

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