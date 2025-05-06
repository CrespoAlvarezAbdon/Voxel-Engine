/**
* @file noise.h
* @version 1.0
* @date 25/04/2023
* @author Abdon Crespo Alvarez
* @title Base class for noise generators.
* @brief Contains the declaration of some the base class for noise generators.
*/
#ifndef _VOXELENG_NOISE_
#define _VOXELENG_NOISE_

#include "definitions.h"


namespace VoxelEng {

	////////////
	//Classes.//
	////////////

	/**
	* @brief Base class for noise generators.
	*/
	class noise {

	public:

		// Constructors.

		/**
		* @brief Default class constructor.
		*/
		noise();


		// Modifiers.

		virtual void setSeed(unsigned int seed);

	protected:

		unsigned int seed_;

	};

	inline noise::noise()
	: seed_(0) {}

	inline void noise::setSeed(unsigned int seed) {
	
		seed_ = seed;
	
	}

}

#endif