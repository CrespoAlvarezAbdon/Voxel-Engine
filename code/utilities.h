/**
* @file utilities.h
* @version 1.0
* @date 25/04/2023
* @author Abdon Crespo Alvarez
* @title Utilities.
* @brief Contains the declaration of some general utility code that
* is not directly related to the engine and could potentially be
* used in other projects easily.
*/
#ifndef _VOXENG_UTILITIES_
#define _VOXENG_UTILITIES_
#include <cstddef>
#include <concepts>
#include <cmath>
#include <string>
#include <vector>
#include <limits>
#include <type_traits>
#include <iostream>
#include <ios>
#include <stdexcept>
#include "definitions.h"
#include "logger.h"
#include "vec.h"

#if GRAPHICS_API == OPENGL

#include <glm.hpp>

#endif


namespace VoxelEng {

	//////////////
	//Functions.//
	//////////////


	/**
	* @brief Computes a % b using floor modulo operations.
	* The equation computed is
	* r = a - b * q
	* where
	* q = (int)((double)a / b)
	*/
	inline int floorMod(int a, int b) {

		return a - b * (int)(std::floor((double)a / b));

	}


	/**
	* @brief Returns +1 if 'real' >= 0 and -1 otherwise.
	*/
	inline int sign(float real) {
	
		return real >= 0 ? 1 : -1;
	
	};


	// vec3 utility functions.

	/**
	* @brief Returns the index of the first maximum value in 'vector'.
	* For example, if vector = 1.0f, 1.0f, 1.0f, indexMax(vector) will return 0, but
	* if vector = 0.0f, 1.0f, 1.0f, it will return 1.
	*/
	unsigned int indMaxVec(const vec3& vector);

	/**
	* @brief Returns 'direction' in a vec3.
	*/
	vec3 uDirectionToVec3(blockViewDir direction);

	/**
	* @brief Returns 'direction' in three floats.
	*/
	void uDirectionToVec3(blockViewDir direction, float& x, float& y, float& z);

	/**
	* @brief Returns 'direction' in as a Block View Direction by rounding to the
	* nearest direction.
	*/
	blockViewDir vec3ToUDirection(const vec3& direction);

	/**
	* @brief Returns the rotated direction.
	*/
	blockViewDir rotateUDirection(blockViewDir dir, blockViewDir rot);

	/**
	* @brief Returns the inverse direction.
	*/
	blockViewDir inverseUDirection(blockViewDir dir);

	/**
	* @brief Returns true if 'string' only contains alphanumeric characters
	* or false otherwise.
	*/
	bool isalnum(const std::string& string);

	template <typename T>
	T sto(const std::string& str) = delete;

	/**
	* @brief Returns the string converted into an integer.
	* Throws std::invalid_argument if could not convert.
	*/
	template <>
	inline int sto(const std::string& str) {
	
		return std::stoi(str);
	
	}

	/**
	* @brief Returns the string converted into an unsigned integer.
	* Throws std::invalid_argument if could not convert.
	*/
	template <>
	unsigned int sto(const std::string& str);

	/**
	* @brief Returns the string converted into a float.
	* Throws std::invalid_argument if could not convert.
	*/
	template <>
	inline float sto(const std::string& str) {

		return std::stof(str);

	}

	/**
	* @brief Returns the string converted into a char.
	* Throws std::invalid_argument if could not convert.
	*/
	template <>
	char sto(const std::string& str);

	/**
	* @brief Returns the string converted into a bool.
	* Throws std::invalid_argument if could not convert.
	*/
	template <>
	inline bool sto(const std::string& str) {

		return std::stoi(str);

	}

	/**
	* @brief Returns the string converted into a numericShortID.
	* Throws std::invalid_argument if could not convert.
	*/
	template <>
	inline unsigned short sto(const std::string& str) {

		return std::stoul(str);

	}

	/**
	* @brief Returns the string converted into a Block View Direction.
	* WARNING. It does not convert, for example, a string "PLUSX" into its Block View Direction
	* counterpart. It is used to get the numbers that represent said directions inside the enum
	* and convert them to the proper type. The objective of this is to parse the directions
	* into simple number of 1 digit in the recording files and parse those numbers into
	* their appropriate enum values.
	* Throws std::invalid_argument if could not convert.
	*/
	template <>
	blockViewDir sto(const std::string& str);

	/**
	* @brief Returns 'value' translated from the range defined in [r1Min, r1Max]
	* to the one defined in [r2Min, r2Max].
	*/
	template <typename T>
	requires std::is_arithmetic<T>::value
	T translateRange(T value, T r1Min, T r1Max, T r2Min, T r2Max) {
	
		if (r1Min < r1Max && r2Min < r2Max)
			return (value - r1Min) * ((r2Max - r2Min) / (r1Max - r1Min)) + r2Min;
		else
			logger::errorLog("The following condition must be true: r1Min < r1Max && r2Min < r2Max");
	
	}

	/**
	* @brief Inserts user input into the desirable value.
	* Requires T to satisfy std::is_arithmetic<T>::value || std::is_same<T, std::string>::value
	*/
	template <typename T>
	requires std::is_arithmetic<T>::value || std::is_same<T, std::string>::value
	bool validatedCinInput(T& var) {

		bool valid = false;

		try {

			std::string s;

			std::getline(std::cin, s);
			std::cin.clear();

			if (s.find_first_not_of("-+.0123456789") != std::string::npos)
				return false;

			var = sto<T>(s);
			valid = true;

		}
		catch (std::invalid_argument const& e) {}
		
		return valid;
	
	}

	/**
	* @brief Inserts user input into the desirable value.
	*/
	template <>
	bool validatedCinInput(std::string& var);

	/**
	* @brief Inserts user input into the desirable value.
	*/
	template <>
	bool validatedCinInput(char& var);

	/**
	* @brief Returns the vector's coordinates in a linear index.
	* NOTE. 'maxY' and 'maxZ' are equal to their respective dimensions' maximum value minus 1.
	*/
	inline unsigned int vec3ToLinear(const vec3& v, int maxY, int maxZ) {
	
		return v.x * maxZ * maxY + v.y * maxZ + v.z;
	
	}

	/**
	* @brief Returns the linear index as vector's coordinates.
	* NOTE. 'maxY' and 'maxZ' are equal to their respective dimensions' maximum value minus 1.
	*/
	inline vec3 linearToVec3(unsigned int linearIndex, int maxY, int maxZ) {
	
		return vec3(linearIndex / (maxZ * maxY), (linearIndex / maxZ) % maxY, linearIndex % maxZ);
	
	}
	/**
	* @brief Returns the vector's coordinates in a linear index.
	* NOTE. 'maxY' is equal to its dimension's maximum value minus 1.
	*/
	inline unsigned int vec2ToLinear(const vec2& v, int maxY) {

		return v.x * maxY + v.y;

	}
	/**
	* @brief Returns the linear index as vector's coordinates.
	* NOTE. 'maxY' is equal to its dimension's maximum value minus 1.
	*/
	inline vec2 linearToVec2(unsigned int linearIndex, int maxY) {

		return vec2(linearIndex / maxY, linearIndex % maxY);

	}

	/**
	* @brief Returns chunk local coordinates of a global position.
	*/
	vec3 getChunkRelCoords(float globalX, float globalY, float globalZ);

	/**
	* @brief Returns chunk local coordinates of a global position.
	*/
	inline vec3 getChunkRelCoords(const vec3& globalPos) {

		return getChunkRelCoords(globalPos.x, globalPos.y, globalPos.z);

	}

	/**
	* @brief Returns chunk local coordinates of a global position.
	*/
	inline vec3 getChunkCoords(float globalX, float globalY, float globalZ) {

		return vec3{ (int)floor(globalX / CHUNK_SIZE), (int)floor(globalY / CHUNK_SIZE), (int)floor(globalZ / CHUNK_SIZE) };

	}

	/**
	* @brief Returns chunk grid coordinates of a global position.
	*/
	inline vec3 getChunkCoords(const vec3& globalPos) {

		return getChunkCoords(globalPos.x, globalPos.y, globalPos.z);

	}

	/**
	* @brief Returns chunk coordinates in the X and Z axes of a global position in the X and Z axes.
	*/
	inline vec2 getChunkXZCoords(int x, int z) {

		return vec2{ (int)floor((double)x / CHUNK_SIZE), (int)floor((double)z / CHUNK_SIZE) };

	}

	/**
	* @brief Returns chunk coordinates in the X and Z axes of a global position in the X and Z axes.
	*/
	inline vec2 getChunkXZCoords(const vec2& blockXZPos) {

		return getChunkXZCoords(blockXZPos.x, blockXZPos.y);

	}

	/**
	* @brief Returns global coordinates of a local chunk coordinate from a certain chunk position.
	*/
	inline vec3 getGlobalPos(int chunkX, int chunkY, int chunkZ, int inChunkX, int inChunkY, int inChunkZ) {

		return vec3{ (float)chunkX * CHUNK_SIZE + inChunkX, (float)chunkY * CHUNK_SIZE + inChunkY, (float)chunkZ * CHUNK_SIZE + inChunkZ };

	}

	/**
	* @brief Returns global coordinates of a local chunk coordinate from a certain chunk position.
	*/
	inline vec3 getGlobalPos(const vec3& chunkPos, const vec3& inChunkPos) {

		return getGlobalPos(chunkPos.x, chunkPos.y, chunkPos.z, inChunkPos.x, inChunkPos.y, inChunkPos.z);

	}

	/**
	* @brief Returns global coordinates of a local chunk coordinate from a certain chunk position.
	*/
	inline vec3 getGlobalPos(const vec3& chunkPos, int inChunkX, int inChunkY, int inChunkZ) {

		return getGlobalPos(chunkPos.x, chunkPos.y, chunkPos.z, inChunkX, inChunkY, inChunkZ);

	}

	/**
	* @brief Returns global coordinates in the X and Z axes of a local chunk coordinate from a certain chunk position,
	* both in the X and Z axes.
	*/
	inline vec2 getXZGlobalPos(int chunkX, int chunkZ, int inChunkX, int inChunkZ) {

		return vec2{ (float)chunkX * CHUNK_SIZE + inChunkX, (float)chunkZ * CHUNK_SIZE + inChunkZ };

	}

	/**
	* @brief Returns global coordinates in the X and Z axes of a local chunk coordinate from a certain chunk position,
	* both in the X and Z axes.
	*/
	inline vec2 getXZGlobalPos(const vec2& chunkPos, const vec2& inChunkPos) {

		return getXZGlobalPos(chunkPos.x, chunkPos.y, inChunkPos.x, inChunkPos.y);

	}

	/**
	* @brief Return the given vector without sign in any of its components.
	*/
	inline vec3 abs(const vec3& v) {
	
		return vec3{ (float)std::abs(v.x), (float)std::abs(v.y), (float)std::abs(v.z) };
	
	}

	/**
	* @brief Encode the given normal vector into a OpenGL's GL_INT_2_10_10_10_REV 32-bit unsigned integer.
	*/
	uint32_t encodeNormalIntoGL_INT_2_10_10_10_REV(float normalX, float normalY, float normalZ);

}

#endif