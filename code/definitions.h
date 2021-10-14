#ifndef _DEFINITIONS_
#define _DEFINITIONS_
#include <GL/glew.h>
#include <GLFW/glfw3.h>


/*
General definitions of auxiliary types, values, structs and classes belong here
*/

namespace VoxelEng 
{

	///////////
	//Structs//
	///////////

	template <typename T>
	struct vec3
	{

		vec3(T x, T y, T z);

		T x,
		  y,
		  z;

	};

	template<typename T>
	vec3<T>::vec3(T x, T y, T z)
		: x(x), y(y), z(z)
	{}

	///////////
	//Defines//
	///////////

	#define Y_CHUNKS_RANGE 12 // How many chunks to load in the y-axis.

	#define SCX 16 // Chunk size in X axis.
	#define SCY 16 // Chunk size in Y axis.
	#define SCZ 16 // Chunk size in Z axis.

	#define MIN_WIDTH 800 // Minimum width for a game window.
	#define MIN_HEIGHT 600 // Minimum height for a game window.


	////////////
	//Typedefs//
	////////////

	typedef unsigned short block; // Block's ID.

	typedef unsigned char byte; // Number with values between 0 and 255.

	typedef GLFWwindow GraphicsAPIWindow;

}

#endif