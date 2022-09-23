#ifndef _VERTEX_
#define _VERTEX_
#include <GL/glew.h>
#include "definitions.h"


namespace VoxelEng {

	///////////////////
	//Classes/Structs//
	///////////////////


	/*
	Represents a 3D vertex with texture coordinates.
	*/
	struct vertex {

		vertexCoord positions[3]; // 0 = coord in X axis, 1 = coord in Y axis and 2 = coord in Z axis.
		textureCoord textureCoords[2];
		// Follows GL_INT_2_10_10_10_REV format. That is, 10 first bits for first normal coord, 
		// the 10 next for the second normal coord... and the last two bits are unused 
		// (for the moment) by alignment reasons.
		GLint normals = 0;

	};

	/*
	Represents a 2D vertex (used for drawing GUI).
	*/
	struct vertex2D {

		vertexCoord positions[2];
		textureCoord textureCoords[2];

	};

}

#endif