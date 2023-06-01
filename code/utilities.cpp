#include "utilities.h"
#include <cstddef>
#include <cctype>
#include "logger.h"


namespace VoxelEng {

	int indMaxAbsVec(const glm::vec3 vec) {
	
		float x = abs(vec.x),
			  y = abs(vec.y),
			  z = abs(vec.z);

		if (x >= y)
			return x >= z ? 0 : 2;
		else
			return y >= z ? 1 : 2;
	
	}

	unsigned int indMaxVec(const vec3& vector) {

		if (vector.x >= vector.y)
			return vector.x >= vector.z ? 0 : 2;
		else
			return vector.y >= vector.z ? 1 : 2;
	
	}

	vec3 uDirectionToVec3(blockViewDir direction) {

		switch (direction) {

			case blockViewDir::PLUSY:
				return vec3FixedUp;
				break;
			case blockViewDir::NEGY:
				return vec3FixedDown;
				break;
			case blockViewDir::PLUSX:
				return vec3FixedNorth;
				break;
			case blockViewDir::NEGX:
				return vec3FixedSouth;
				break;
			case blockViewDir::PLUSZ:
				return vec3FixedEast;
				break;
			case blockViewDir::NEGZ:
				return vec3FixedWest;
				break;
			default:
				logger::errorLog("Undefined direction specified when converting to vec3");
				break;

		}
	
	}

	blockViewDir vec3ToUDirection(const vec3& direction) {
	
		unsigned int iMax = indMaxVec(direction);

		if (iMax == 0) // x
			return direction.x >= 0 ? blockViewDir::PLUSX : blockViewDir::NEGX;
		else if (iMax == 1) // y
			return direction.y >= 0 ? blockViewDir::PLUSY : blockViewDir::NEGY;
		else // z
			return direction.z >= 0 ? blockViewDir::PLUSZ : blockViewDir::NEGZ;
	
	}

	// Básate aquí para hacer el rotateView
	blockViewDir rotateUDirection(blockViewDir dir, blockViewDir rot) {
	
		switch (dir) {

		case blockViewDir::PLUSX:

			switch (rot) {

			case blockViewDir::PLUSX:
				return blockViewDir::PLUSZ;
				break;

			case blockViewDir::NEGX:
				return blockViewDir::NEGZ;
				break;

			case blockViewDir::PLUSY:
				return blockViewDir::PLUSY;
				break;

			case blockViewDir::NEGY:
				return blockViewDir::NEGY;
				break;

			default:
				logger::errorLog("Invalid specified uDirection rotation");
				break;

			}

			break;

		case blockViewDir::NEGX:

			switch (rot) {

			case blockViewDir::PLUSX:
				return blockViewDir::NEGZ;
				break;

			case blockViewDir::NEGX:
				return blockViewDir::PLUSZ;
				break;

			case blockViewDir::PLUSY:
				return blockViewDir::PLUSY;
				break;

			case blockViewDir::NEGY:
				return blockViewDir::NEGY;
				break;

			default:
				logger::errorLog("Invalid specified uDirection rotation");
				break;

			}

			break;

		case blockViewDir::PLUSY:

			switch (rot) {

			case blockViewDir::PLUSX:
				return blockViewDir::PLUSZ;
				break;

			case blockViewDir::NEGX:
				return blockViewDir::NEGZ;
				break;

			case blockViewDir::PLUSY:
				return blockViewDir::NEGX;
				break;

			case blockViewDir::NEGY:
				return blockViewDir::PLUSX;
				break;

			default:
				logger::errorLog("Invalid specified uDirection rotation");
				break;

			}

			break;

		case blockViewDir::NEGY:

			switch (rot) {

			case blockViewDir::PLUSX:
				return blockViewDir::PLUSZ;
				break;

			case blockViewDir::NEGX:
				return blockViewDir::NEGZ;
				break;

			case blockViewDir::PLUSY:
				return blockViewDir::PLUSX;
				break;

			case blockViewDir::NEGY:
				return blockViewDir::NEGX;
				break;

			default:
				logger::errorLog("Invalid specified uDirection rotation");
				break;

			}

			break;

		case blockViewDir::PLUSZ:

			switch (rot) {

			case blockViewDir::PLUSX:
				return blockViewDir::NEGX;
				break;

			case blockViewDir::NEGX:
				return blockViewDir::PLUSX;
				break;

			case blockViewDir::PLUSY:
				return blockViewDir::PLUSY;
				break;

			case blockViewDir::NEGY:
				return blockViewDir::NEGY;
				break;

			default:
				logger::errorLog("Invalid specified uDirection rotation");
				break;

			}

			break;

		case blockViewDir::NEGZ:

			switch (rot) {

			case blockViewDir::PLUSX:
				return blockViewDir::PLUSX;
				break;

			case blockViewDir::NEGX:
				return blockViewDir::NEGX;
				break;

			case blockViewDir::PLUSY:
				return blockViewDir::PLUSY;
				break;

			case blockViewDir::NEGY:
				return blockViewDir::NEGY;
				break;

			default:
				logger::errorLog("Invalid specified uDirection rotation");
				break;

			}

			break;

		default:
			logger::errorLog("Invalid specified uDirection to rotate");
			break;

		}
	
	}

	blockViewDir inverseUDirection(blockViewDir dir) {
	
		switch (dir) {

		case blockViewDir::PLUSX:

			return blockViewDir::NEGX;
			break;

		case blockViewDir::NEGX:

			return blockViewDir::PLUSX;
			break;

		case blockViewDir::PLUSY:

			return blockViewDir::NEGY;
			break;

		case blockViewDir::NEGY:

			return blockViewDir::PLUSY;
			break;

		case blockViewDir::PLUSZ:

			return blockViewDir::NEGZ;
			break;

		case blockViewDir::NEGZ:

			return blockViewDir::PLUSZ;
			break;

		}
	
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
			logger::errorOutOfRange(" Thrown when trying to convert '" + str + "' into an unsigned integer");

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
	block sto(const std::string& str) {

		unsigned long result = std::stoul(str);

		if (result > std::numeric_limits<block>::max())
			logger::errorOutOfRange(" Thrown when trying to convert '" + str + "' into an block ID");

		return result;

	}

	template <>
	blockViewDir sto(const std::string& str) {

		unsigned long value = std::stoul(str);

		if (value > std::numeric_limits<block>::max())
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

}