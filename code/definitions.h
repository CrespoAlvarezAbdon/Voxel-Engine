#ifndef _DEFINITIONS_
#define _DEFINITIONS_

namespace VoxelEng 
{

	// Block's ID.
	typedef unsigned short block;

	// Number with values between 0 and 255.
	typedef signed char byte;

	// How many chunks to load in the y-axis.
	#define Y_CHUNKS_RANGE 12

	// Chunk size in X axis.
	#define SCX 16
	// Chunk size in Y axis.
	#define SCY 16
	// Chunk size in Z axis.
	#define SCZ 16

}

#endif