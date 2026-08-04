#ifndef _VOXELENG_LIGHTING_DEFINITIONS_
#define _VOXELENG_LIGHTING_DEFINITIONS_

namespace VoxelEng {

	/////////////
	//Typedefs.//
	/////////////

	/**
	* @brief Light intensity. It determines how much further the light will expand for each channel. Range is [0, 8].
	*/
	typedef byte lightIntensity;

	/**
	* @brief Value for a light channel. Range is [-255, 255].
	*/
	typedef unsigned short lightValue;


	/////////////////
	//Enum classes.//
	/////////////////

	/**
	* @brief Color channels.
	* RED = Red color channel.
	* GREEN = Green color channel.
	* BLUE = Blue color channel.
	* ALPHA = Alpha color channel.
	* RGB = All RGB color channels.
	* ALL = All RGBA color channels.
	*/
	enum class colorChannel {RED = 0, GREEN, BLUE, ALPHA, RGB, ALL};


	//////////////
	//Constants.//
	//////////////

	/**
	* @brief Default maximum intensity that a light source can emit to.
	* Light intensity is decreased minimally by one per block travelled, so it also dictactes the
	* maximum distance travelled by light from its source block.
	*/
	const lightIntensity kLightMaxIntensity = 8;

}

#endif