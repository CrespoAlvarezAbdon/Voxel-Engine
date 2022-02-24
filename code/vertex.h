#ifndef _VERTEX_
#define _VERTEX_
#include <GL/glew.h>
#include "definitions.h"


///////////
//Structs//
///////////



/*
Represents a 3D vertex with texture coordinates.
*/
struct vertex
{

	VoxelEng::vertexCoord positions[3]; // 0 = coord in X axis, 1 = coord in Y axis and 2 = coord in Z axis.
	VoxelEng::textureCoord textureCoords[2];
	// Follows GL_INT_2_10_10_10_REV format. That is, 10 first bits for first normal coord, 
	// the 10 next for the second normal coord... and the last two bits are unused 
	// (for the moment) by alignment reasons.
	VoxelEng::normalVec normals; 

};

#endif