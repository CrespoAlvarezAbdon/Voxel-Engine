#ifndef _VOXENG_UTILITIES_
#define _VOXENG_UTILITIES_
#include <glm.hpp>
#include <string>
#include <vector>
#include <limits>
#include <cstddef>
#include <concepts>
#include <type_traits>
#include <iostream>
#include <ios>
#include <limits>
#include <cmath>
#include "definitions.h"
#include "logger.h"


namespace VoxelEng {

	//////////////
	//Functions.//
	//////////////


	/*
	Computes a % b using floor modulo operations.
	The equation computed is
	r = a - b * q
	where
	q = (int)((double)a / b)
	*/
	inline int floorMod(int a, int b) {

		return a - b * (int)(std::floor((double)a / b));

	}


	/*
	Returns +1 if 'real' >= 0 and -1 otherwise.
	*/
	inline int sign(float real) {
	
		return real >= 0 ? 1 : -1;
	
	};


	// vec3 utility functions.

	/*
	Returns the index of the first maximum value in 'vector'.
	For example, if vector = 1.0f, 1.0f, 1.0f, indexMax(vector) will return 0, but
	if vector = 0.0f, 1.0f, 1.0f, it will return 1.
	*/
	unsigned int indMaxVec(const vec3& vector);

	vec3 uDirectionToVec3(unsigned int direction);

	unsigned int vec3ToUDirection(const vec3& direction);

	unsigned int rotateUDirection(unsigned int dir, unsigned int rot);

	unsigned int inverseUDirection(unsigned int dir);

	bool isalnum(const std::string& string);

	template <typename T>
	T sto(const std::string& str) = delete;

	template <>
	inline int sto(const std::string& str) {
	
		return std::stoi(str);
	
	}

	template <>
	unsigned int sto(const std::string& str);

	template <>
	inline float sto(const std::string& str) {

		return std::stof(str);

	}

	template <>
	char sto(const std::string& str);

	template <>
	inline bool sto(const std::string& str) {

		return std::stoi(str);

	}

	template <>
	block sto(const std::string& str);

	/*
	Returns 'value' translated from the range defined in [r1Min, r1Max]
	*/
	template <typename T>
	requires std::is_arithmetic<T>::value
	T translateRange(T value, T r1Min, T r1Max, T r2Min, T r2Max) {
	
		if (r1Min < r1Max && r2Min < r2Max)
			return (value - r1Min) * ((r2Max - r2Min) / (r1Max - r1Min)) + r2Min;
		else
			logger::errorLog("The following condition must be true: r1Min < r1Max && r2Min < r2Max");
	
	}

	/*
	Inserts user input into the desirable value.
	Requires T to satisfy std::is_arithmetic<T>::value || std::is_same<T, std::string>::value
	*/
	template <typename T>
	requires std::is_arithmetic<T>::value || std::is_same<T, std::string>::value
	bool validatedCinInput(T& var) {
	
		bool valid = false;
		if (std::cin >> var)
			valid = true;

		std::cin.clear();
		std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
		
		return valid;
	
	}

}

#endif