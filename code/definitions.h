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

#include <vector>
#include <chrono>
#include <string>

// Definitions.
#define GRAPHICS_API OPENGL

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
	* @brief Chunk size (in blocks) in X axis.
	*/
	const int SCXLimit = SCX-1;

	/**
	* @brief Chunk size (in blocks) in Y axis.
	*/
	const int SCYLimit = SCY-1;

	/**
	* @brief Chunk size (in blocks) in Z axis.
	*/
	const int SCZLimit = SCZ-1;

	/**
	* @brief The total number of blocks per chunk.
	*/
	const int nBlocksChunk = SCX * SCY * SCZ;

	/**
	* @brief The total number of blocks per chunk edge.
	*/
	const int nBlocksChunkEdge = SCX * SCY;

	/**
	* @brief Number of chunks to compute in the Y axis (only in the +Y direction or the -Y direction).
	*/
	const int yChunksRange = 12;

	/**
	* @brief Total number of chunks to compute in the Y axis (both in the +Y direction and the -Y direction).
	*/
	const int totalYChunks = yChunksRange * 2;

	/**
	* @brief Maximum distance in chunk coordinates from the player for anything rendered in LOD level 1 (100% resolution).
	*/
	const int LODlevel1Range = 10;

	/**
	* @brief Maximum distance in chunk coordinates from the player for anything rendered in LOD level 2 (50% resolution).
	*/
	const int LODlevel2Range = 20;

	/**
	* @brief Default width for a game window.
	*/
	const unsigned int DEF_WIDTH = 800;

	/**
	* @brief Default height for a game window.
	*/
	const unsigned int DEF_HEIGHT = 600;

	/**
	* @brief Default maximum distance in chunk coordinates for chunks to be computed in the X and Z axes.
	*/
	const unsigned int DEF_N_CHUNKS_TO_COMPUTE = 20;

	/**
	* @brief Total number of blocks to compute taking into account the total amount of chunks to compute and the size of a chunk in blocks.
	*/
	const unsigned int totalNBlocksToCompute = DEF_N_CHUNKS_TO_COMPUTE*2 * DEF_N_CHUNKS_TO_COMPUTE*2 * totalYChunks * SCX*SCY*SCZ;

	/**
	* @brief Number of GUIelement layers in which to organize the graphical user interface.
	*/
	const unsigned int N_GUI_LAYERS = 3;

	/**
	* @brief Maximum number of chunk generation/meshing simultaneous jobs being executed.
	*/
	const unsigned int MAX_N_CHUNK_SIMULT_TASKS = 2;

	const float piDiv = 3.1415926f / 180.0f;


	/////////////////////
	//Type definitions.//
	/////////////////////

	typedef void(*tickFunc)();

	typedef unsigned int agentID;
	typedef unsigned int entityID;

	typedef std::string namespacedID;
	typedef unsigned short numericShortID;
	typedef unsigned int numID;

	typedef unsigned char byte; // Number with values between 0 and 255.

	typedef float vertexCoord;

	typedef float textureCoord;

	typedef float angle;

	#if GRAPHICS_API == OPENGL

		typedef GLint normalVec;

		typedef GLFWwindow GraphicsAPIWindow;

	#endif

	typedef std::chrono::time_point<std::chrono::high_resolution_clock> timePoint;
	typedef long long duration;

}

#endif