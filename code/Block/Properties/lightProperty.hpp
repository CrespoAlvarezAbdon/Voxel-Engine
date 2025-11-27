#ifndef _VOXELENG_LIGHT_PROPERTY_
#define _VOXELENG_LIGHT_PROPERTY_

#include <Block/Properties/blockProperty.hpp>
#include <vec.h>

namespace VoxelEng {

	struct lightProperty : public blockProperty {

	public:

		/*
		Methods.
		*/

		// Constructors.

		/**
		* @brief Default class constructor.
		*/
		lightProperty();

		/**
		* @brief Class constructor.
		* @param intensity Light's intensity level (0-15).
		* @param color Light's color (rgba, -128 to 127 for each channel).
		*/
		lightProperty(char intensity, const basicVec4& color);


		/*
		Attributes.
		*/

		/**
		* @brief It determines whether the light's color is applied fully in its position or not.
		*/
		char intensity;

		/**
		* Light's color applied to its position. Last value is alpha.
		*/
		basicVec4 color;

	};

	inline lightProperty::lightProperty() 
	: blockProperty(), intensity(0), color(basicVec4Zero)
	{}

	inline lightProperty::lightProperty(char intensity, const basicVec4& color)
	: blockProperty(), intensity(intensity), color(color)
	{}

}

#endif