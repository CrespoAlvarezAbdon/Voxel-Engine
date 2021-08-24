#ifndef _VERTEX_
#define _VERTEX_
#include <GL/glew.h>

// TODO
// REMOVE THIS DEFINITION.
typedef unsigned int Index;


///////////
//Structs//
///////////

/*
Represents a 3D vertex with texture coordinates.
*/
struct vertex
{

	GLbyte positions[3];
	GLbyte dummy;
	GLfloat textureCoords[2];

};

#endif