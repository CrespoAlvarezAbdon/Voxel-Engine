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

	VoxelEng::byte positions[3];
	VoxelEng::byte dummy;
	float textureCoords[2];

};

#endif