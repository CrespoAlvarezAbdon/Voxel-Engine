#ifndef _VOXELENG_LIGHTING_DEFINITIONS_
#define _VOXELENG_LIGHTING_DEFINITIONS_

namespace VoxelEng {

	/**
	* @brief Light intensity. It determines how much further the light will expand for each channel. Range is [0, 8].
	*/
	typedef byte lightIntensity;

	/**
	* @brief Value for a light channel. Range is [-127, 128].
	*/
	typedef sbyte lightValue;

	/**
	* @brief Color channels.
	* RED = Red color channel.
	* GREEN = Green color channel.
	* BLUE = Blue color channel.
	* ALPHA = Alpha color channel.
	* ALL = All RGBA color channels.
	* RGB = Only RGB color channels.
	*/
	enum class colorChannel {RED = 0, GREEN, BLUE, ALPHA, ALL, RGB};

}

#endif