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

		// Public variables.

		T x,
		  y,
	      z;


		// Constructors.

		vec3(const T& x, const T& y, const T& z);

		vec3();

	};

	template<typename T>
	vec3<T>::vec3(const T& x, const T& y, const T& z)
		: x(x), y(y), z(z)
	{}

	template<typename T>
	vec3<T>::vec3()
		: x(0), y(0), z(0)
	{}

	template <typename T>
	struct vec2
	{

		// Public variables.

		T x,
		  y;


		// Constructors.

		vec2(const T& x, const T& y);

		vec2();

	};

	template<typename T>
	vec2<T>::vec2(const T& x, const T& y)
		: x(x), y(y)
	{}

	template<typename T>
	vec2<T>::vec2()
		: x(0), y(0)
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

	// Directions.
	#define UP 1 // +y
	#define DOWN 2 // -y
	#define NORTH 3 // +x
	#define SOUTH 4 // -x
	#define EAST 5 // +z
	#define WEST 6 // -z

	// Axis.
	#define X_AXIS 1
	#define Y_AXIS 2
	#define Z_AXIS 3


	////////////
	//Typedefs//
	////////////

	typedef unsigned short block; // Block's ID.

	typedef unsigned char byte; // Number with values between 0 and 255.

	typedef float vertexCoord;

	typedef float textureCoord;

	typedef int normalVec;

	typedef float angle;

	// TODO. ADD CONDITIONS TO ALLOW SWITCHING BETWEEN GRAPHIC APIs
	typedef GLFWwindow GraphicsAPIWindow;

}

#endif