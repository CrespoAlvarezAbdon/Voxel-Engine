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

#include <concepts>
#include <vector>
#include <chrono>
#include <string>

////////////////
//Definitions.//
////////////////
#define GRAPHICS_API OPENGL

#if GRAPHICS_API == OPENGL

	#include <GL/glew.h>
	#include <GLFW/glfw3.h>
	#include <glm.hpp>

#endif


namespace VoxelEng {

	/////////////
	//Concepts.//
	/////////////

	template <typename T>
	concept dsefaultConstructible = requires {

		T();

	};


	/////////////////
	//Enum classes.//
	/////////////////

	/**
	* @brief The different graphics APIs supported by the engine.
	*/
	enum class graphicsAPI { OPENGL };


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
	* @brief Chunk size (in blocks) in X, Y AND Z axes.
	*/
	const int CHUNK_SIZE = 16;

	/**
	* @brief Size of chunk C (in blocks) in X, Y AND Z axes but also counting the blocks from the neighboring chunks
	* that C has also need to take into account for.
	*/
	const int chunkSizePlusNeighbors = CHUNK_SIZE + 1;

	/**
	* @brief Chunk size (in blocks) in X axis.
	*/
	const int CHUNK_SIZE_LIMIT = CHUNK_SIZE-1;

	/**
	* @brief The total number of blocks per chunk.
	*/
	const int nBlocksChunk = CHUNK_SIZE * CHUNK_SIZE * CHUNK_SIZE;

	/**
	* @brief The total number of blocks per chunk C but also counting the blocks from the neighboring chunks
	* that C has also need to take into account for.
	*/
	const int nBlocksChunkPlusNeighbors = chunkSizePlusNeighbors * chunkSizePlusNeighbors * chunkSizePlusNeighbors;

	/**
	* @brief The total number of blocks per chunk edge.
	*/
	const int nBlocksChunkEdge = CHUNK_SIZE * CHUNK_SIZE;

	/**
	* @brief Number of chunks to compute in the Y axis (only in the +Y direction or the -Y direction).
	*/
	const int yChunksRange = 12;

	/**
	* @brief Total number of chunks to compute in the Y axis (both in the +Y direction and the -Y direction).
	*/
	const int totalYChunks = yChunksRange * 2;

	/**
	* @brief Default width for a game window.
	*/
	const unsigned int DEF_WIDTH = 800;

	/**
	* @brief Default height for a game window.
	*/
	const unsigned int DEF_HEIGHT = 600;

	/**
	* @brief Number of GUIelement layers in which to organize the graphical user interface.
	*/
	const unsigned int N_GUI_LAYERS = 3;

	/**
	* @brief Approximation of pi divided by 180.
	*/
	const float piDiv = 3.1415926f / 180.0f;

	/**
	* @brief The number of neighbors a chunk has.
	*/
	const int nChunkNeighbors = 26;

	/**
	* @brief Number of neigbors that are directly colliding with the block.
	*/
	const int N_BLOCK_DIRECT_NEIGHBORS = 6;


	/////////////////////
	//Type definitions.//
	/////////////////////

	typedef void(*tickFunc)();

	typedef unsigned int agentID;
	typedef unsigned int entityID;

	typedef std::string namespacedID;
	typedef unsigned short numericShortID;
	typedef unsigned int numID;

	typedef uint8_t byte; // Number with values between 0 and 255.

	typedef int8_t sbyte;

	typedef float vertexCoord;

	typedef float textureCoord;

	typedef float angle;

	#if GRAPHICS_API == OPENGL

		typedef GLint normalVec;

		typedef GLFWwindow GraphicsAPIWindow;

	#endif

	typedef std::chrono::time_point<std::chrono::high_resolution_clock> timePoint;
	typedef long long duration;

	/**
	* Provides a literal char definition.
	*/
	constexpr char operator"" _c(unsigned long long v) {

		return static_cast<char>(v);

	}

	/**
	* Provides a literal unsigned char definition.
	*/
	constexpr unsigned char operator"" _uc(unsigned long long v) {

		return static_cast<unsigned char>(v);

	}

}

#endif