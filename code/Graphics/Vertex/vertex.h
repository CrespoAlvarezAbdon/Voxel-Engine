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

		float positions[3] = {0,0,0}; // 0 = coord in X axis, 1 = coord in Y axis and 2 = coord in Z axis.
		float textureCoords[2] = {0,0};
		unsigned char color[4] = {255,255,255,255}; // RGBA stored in 32-bits.
		unsigned char additionalData[4] = {0,0,0,0}; // First byte is material index. The next three are the block in-chunk coordinates corresponding to the vertex.
		//normalVec normals;

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