#ifndef _VOXELENG_LIGHT_DATA_
#define _VOXELENG_LIGHT_DATA_

#include <vec.h>

namespace VoxelEng {

	struct lightData {

		/*
		Methods.
		*/

		// Constructors.

		/**
		* @brief Default class constructor.
		*/
		lightData();

		/**
		* @brief Class constructor.
		* @param intensity Light's intensity level (0-15).
		* @param color Light's color (-128 to 127 rgba).
		*/
		lightData(char intensity, const basicVec4& color);


		/*
		Attributes.
		*/

		char intensity;
		basicVec4 color;

	};

	inline lightData::lightData()
	: intensity(0), color(basicVec4Zero)
	{}

	inline lightData::lightData(char intensity, const basicVec4& color)
	: intensity(intensity), color(color)
	{}

}

#endif