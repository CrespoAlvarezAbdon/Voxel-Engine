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

	VoxelEng::vertexCoord positions[3];
	VoxelEng::textureCoord textureCoords[2];

};

#endif