#ifndef _VERTEX_
#define _VERTEX_
#include <GL/glew.h>

typedef unsigned int Index;

struct Block_vertex
{

	GLbyte positions[3]; // X, Y, Z. Z = 0 for 2D. Z is for depth
	GLbyte face_direction;
	GLfloat texture_coordinates[2];

};

#endif