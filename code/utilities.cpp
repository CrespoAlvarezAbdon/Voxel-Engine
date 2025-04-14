#include "utilities.h"
#include <cstddef>
#include <cctype>


namespace VoxelEng {

	unsigned int indMaxVec(const vec3& vector) {

		if (vector.x >= vector.y)
			return vector.x >= vector.z ? 0 : 2;
		else
			return vector.y >= vector.z ? 1 : 2;
	
	}

	

	bool isalnum(const std::string& string) {
	
		for (size_t i = 0; i < string.size(); i++) {
		
			if (!std::isalnum(string[i]))
				return false;
		
		}

		return true;
	
	}

	template <>
	unsigned int sto(const std::string& str) {

		unsigned long result = std::stoul(str);

		if (result > std::numeric_limits<unsigned int>::max())
			logger::errorOutOfRange(str + "' exceeds unsigned integer range");

		return result;

	}

	template <>
	char sto(const std::string& str) {

		if (str.size() == 1)
			return str[0];
		else
			logger::errorLog("A string being converted to a character cannot contain more than one character");

	}

	template <>
	blockViewDir sto(const std::string& str) {

		unsigned long value = std::stoul(str);

		if (value > 6)
			logger::errorOutOfRange(" Thrown when trying to convert '" + str + "' into an blockViewDir");

		return static_cast<blockViewDir>(value);

	}

	/*
	Inserts user input into the desirable value.
	Requires T to satisfy std::is_arithmetic<T>::value || std::is_same<T, std::string>::value
	*/
	template <>
	bool validatedCinInput(std::string& var) {

		std::getline(std::cin, var);
		std::cin.clear();

		return true;

	}

	/*
	Inserts user input into the desirable value.
	Requires T to satisfy std::is_arithmetic<T>::value || std::is_same<T, std::string>::value
	*/
	template <>
	bool validatedCinInput(char& var) {

		std::string s;
		std::getline(std::cin, s);
		std::cin.clear();

		if (s.size() != 1)
			return false;
		else
		{

			var = s[0];

			return true;

		}
	}

	vec3 getChunkRelCoords(float globalX, float globalY, float globalZ) {

		int floorX = std::floor(globalX),
			floorY = std::floor(globalY),
			floorZ = std::floor(globalZ);

		return vec3{ (int)floorMod((floorX >= 0) ? floorX : CHUNK_SIZE + floorX, CHUNK_SIZE),
					 (int)floorMod((floorY >= 0) ? floorY : CHUNK_SIZE + floorY, CHUNK_SIZE),
					 (int)floorMod((floorZ >= 0) ? floorZ : CHUNK_SIZE + floorZ, CHUNK_SIZE) };

	}

	vec3 getRegionRelCoords(float chunkX, float chunkY, float chunkZ) {

		int floorX = std::floor(chunkX),
			floorY = std::floor(chunkY),
			floorZ = std::floor(chunkZ);

		return vec3{ (int)floorMod((floorX >= 0) ? floorX : CHUNK_SIZE + floorX, CHUNK_SIZE),
					 (int)floorMod((floorY >= 0) ? floorY : CHUNK_SIZE + floorY, CHUNK_SIZE),
					 (int)floorMod((floorZ >= 0) ? floorZ : CHUNK_SIZE + floorZ, CHUNK_SIZE) };

	}

	uint32_t packNormalIntoGL_INT_2_10_10_10_REV(float normalX, float normalY, float normalZ) {

		// Convert from [-1.0, 1.0] normal component range to [-512, 511] 10-bit range.
		int32_t x = static_cast<int32_t>(normalX * 511.0f);
		int32_t y = static_cast<int32_t>(normalY * 511.0f);
		int32_t z = static_cast<int32_t>(normalZ * 511.0f);
		int32_t w = 0; // Unused space.

		// Put the bits in their proper places according to OpenGL specification.
		return (w & 0x3) | ((z & 0x3FF) << 2) | ((y & 0x3FF) << 12) | ((x & 0x3FF) << 22);

	}

}