/**
* @file definitions.h
* @version 1.0
* @date 20/04/2023
* @author Abdon Crespo Alvarez
* @title Definitions.
* @brief Contains general typedefs, constant definitions an declarations
* of several other concepts used all across the engine's code.
*/
#ifndef _VOXELENG_DEFINITIONS_
#define _VOXELENG_DEFINITIONS_
#include <chrono>

#if GRAPHICS_API == OPENGL

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm.hpp>

#endif


namespace VoxelEng {

	/////////////////
	//Enum classes.//
	/////////////////

	/**
	* @brief The different graphics APIs supported by the engine.
	*/
	enum class graphicsAPI {OPENGL};

	/**
	* @brief Viewing directions expressed in a discrete and simple way. It could be seen as a "blocky" viewing direction
	* if it could be compared to block coordinates, which are only integer coordinates compared to global coordinates.
	*/
	enum class blockViewDir {NONE, PLUSY, NEGY, PLUSX, NEGX, PLUSZ, NEGZ};


	//////////////
	//Constants.//
	//////////////

	/**
	* @brief The graphics API currently used by the engine during it's execution.
	*/
	const graphicsAPI graphicsAPIUsed = graphicsAPI::OPENGL;

	/**
	* @brief Minimal block texture size in pixels.
	*/
	const unsigned int MIN_TEX_RES = 16;

	/**
	* @brief Chunk size (in blocks) in X axis.
	*/
	const int SCX = 16;

	/**
	* @brief Chunk size (in blocks) in Y axis.
	*/
	const int SCY = 16;

	/**
	* @brief Chunk size (in blocks) in Z axis.
	*/
	const int SCZ = 16;

	/**
	* @brief Default width for a game window.
	*/
	const unsigned int DEF_WIDTH = 800; // 

	/**
	* @brief Default height for a game window.
	*/
	const unsigned int DEF_HEIGHT = 600;

	/**
	* @brief Default total number of chunks to compute in the X and Z axes.
	*/
	const unsigned int DEF_N_CHUNKS_TO_COMPUTE = 10;

	/**
	* @brief Number of GUIelement layers in which to organize the graphical user interface.
	*/
	const unsigned int N_GUI_LAYERS = 3;


	/////////////////////
	//Type definitions.//
	/////////////////////

	typedef void(*tickFunc)();

	#if GRAPHICS_API == OPENGL
	
		typedef glm::vec3 vec3;
		typedef glm::vec2 vec2;

	#else

	

	#endif

	typedef unsigned int agentID;
	typedef unsigned int entityID;

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

	/**
	* @brief Number of chunks to compute in the Y axis (only in the +Y direction or the -Y direction).
	*/
	const int yChunksRange = 12;

	/**
	* @brief Total number of chunks to compute in the Y axis (both in the +Y direction and the -Y direction).
	*/
	const int totalYChunks = yChunksRange * 2;

	/**
	* @brief vec3 constant of the zero vector.
	*/
	const vec3 vec3Zero(0, 0, 0);
	/**
	* @brief vec3 constant poiting to the fixed up direction.
	*/
	const vec3 vec3FixedUp(0, 1, 0);
	/**
	* @brief vec3 constant poiting to the fixed down direction.
	*/
	const vec3 vec3FixedDown(0, -1, 0);
	/**
	* @brief vec3 constant poiting to the fixed north direction.
	*/
	const vec3 vec3FixedNorth(1, 0, 0);
	/**
	* @brief vec3 constant poiting to the fixed south direction.
	*/
	const vec3 vec3FixedSouth(-1, 0, 0);
	/**
	* @brief vec3 constant poiting to the fixed east direction.
	*/
	const vec3 vec3FixedEast(0, 0, 1);
	/**
	* @brief vec3 constant poiting to the fixed west direction.
	*/
	const vec3 vec3FixedWest(0, 0, -1);

}

#endif