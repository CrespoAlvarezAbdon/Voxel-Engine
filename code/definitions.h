#ifndef _VOXELENG_DEFINITIONS_
#define _VOXELENG_DEFINITIONS_
#include <chrono>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm.hpp>


/*
General definitions of auxiliary types, values, structs and classes belong here
*/

namespace VoxelEng {

	////////////
	//Defines.//
	////////////

	// 0 = OPENGL
	// 1 = DIRECTX
	#define OPENGL 0
	#define DIRECTX 1
	#define GRAPHICS_API OPENGL

	#define MIN_TEX_RES 16

	#define SCX 16 // Chunk size in X axis.
	#define SCY 16 // Chunk size in Y axis.
	#define SCZ 16 // Chunk size in Z axis.

	#define MIN_WIDTH 800 // Minimum width for a game window.
	#define MIN_HEIGHT 600 // Minimum height for a game window.

	// Directions.
	#define PLUSY 1 // +y
	#define NEGY 2 // -y
	#define PLUSX 3 // +x
	#define NEGX 4 // -x
	#define PLUSZ 5 // +z
	#define NEGZ 6 // -z

	// Number of GUI layers
	#define	N_GUI_LAYERS 3


	/////////////////////
	//Type definitions.//
	/////////////////////

	typedef void(*tickFunc)();

	#if GRAPHICS_API == OPENGL
	
		typedef glm::vec3 vec3;
		typedef glm::vec2 vec2;

	#else

	

	#endif

	typedef unsigned short block; // Block's ID.

	typedef unsigned char byte; // Number with values between 0 and 255.

	typedef float vertexCoord;

	typedef float textureCoord;

	typedef float angle;

	#if GRAPHICS_API == OPENGL

		typedef GLint normalVec;

		typedef GLFWwindow GraphicsAPIWindow;

	#else

	
	#endif

	typedef std::chrono::time_point<std::chrono::high_resolution_clock> timePoint;
	typedef long long duration;


	//////////////
	//Constants.//
	//////////////
	const unsigned int yChunksRange = 12; // How many chunks to load in the y-axis.
	const int totalYChunks = yChunksRange * 2;
	const vec3 vec3Zero(0, 0, 0);
	const vec3 vec3FixedUp(0, 1, 0);
	const vec3 vec3FixedDown(0, -1, 0);
	const vec3 vec3FixedNorth(1, 0, 0);
	const vec3 vec3FixedSouth(-1, 0, 0);
	const vec3 vec3FixedEast(0, 0, 1);
	const vec3 vec3FixedWest(0, 0, -1);

}

#endif