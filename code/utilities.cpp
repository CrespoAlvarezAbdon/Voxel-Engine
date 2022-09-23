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

	vec3 uDirectionToVec3(unsigned int direction) {

		switch (direction) {

			case PLUSY:
				return vec3FixedUp;
				break;
			case NEGY:
				return vec3FixedDown;
				break;
			case PLUSX:
				return vec3FixedNorth;
				break;
			case NEGX:
				return vec3FixedSouth;
				break;
			case PLUSZ:
				return vec3FixedEast;
				break;
			case NEGZ:
				return vec3FixedWest;
				break;
			default:
				logger::errorLog("Undefined direction specified when converting to vec3");
				break;

		}
	
	}

	unsigned int vec3ToUDirection(const vec3& direction) {
	
		unsigned int iMax = indMaxVec(direction);

		if (iMax == 0) // x
			return direction.x >= 0 ? PLUSX : NEGX;
		else if (iMax == 1) // y
			return direction.y >= 0 ? PLUSY : NEGY;
		else // z
			return direction.z >= 0 ? PLUSZ : NEGZ;
	
	}

	unsigned int rotateUDirection(unsigned int dir, unsigned int rot) {
	
		switch (dir) {

		case PLUSX:

			switch (rot) {

			case PLUSX:
				return PLUSZ;
				break;

			case NEGX:
				return NEGZ;
				break;

			case PLUSY:
				return PLUSY;
				break;

			case NEGY:
				return NEGY;
				break;

			default:
				logger::errorLog("Invalid specified uDirection rotation");
				break;

			}

			break;

		case NEGX:

			switch (rot) {

			case PLUSX:
				return NEGZ;
				break;

			case NEGX:
				return PLUSZ;
				break;

			case PLUSY:
				return PLUSY;
				break;

			case NEGY:
				return NEGY;
				break;

			default:
				logger::errorLog("Invalid specified uDirection rotation");
				break;

			}

			break;

		case PLUSY:

			switch (rot) {

			case PLUSX:
				return PLUSZ;
				break;

			case NEGX:
				return NEGZ;
				break;

			case PLUSY:
				return NEGX;
				break;

			case NEGY:
				return PLUSX;
				break;

			default:
				logger::errorLog("Invalid specified uDirection rotation");
				break;

			}

			break;

		case NEGY:

			switch (rot) {

			case PLUSX:
				return PLUSZ;
				break;

			case NEGX:
				return NEGZ;
				break;

			case PLUSY:
				return PLUSX;
				break;

			case NEGY:
				return NEGX;
				break;

			default:
				logger::errorLog("Invalid specified uDirection rotation");
				break;

			}

			break;

		case PLUSZ:

			switch (rot) {

			case PLUSX:
				return NEGX;
				break;

			case NEGX:
				return PLUSX;
				break;

			case PLUSY:
				return PLUSY;
				break;

			case NEGY:
				return NEGY;
				break;

			default:
				logger::errorLog("Invalid specified uDirection rotation");
				break;

			}

			break;

		case NEGZ:

			switch (rot) {

			case PLUSX:
				return PLUSX;
				break;

			case NEGX:
				return NEGX;
				break;

			case PLUSY:
				return PLUSY;
				break;

			case NEGY:
				return NEGY;
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

	unsigned int inverseUDirection(unsigned int dir) {
	
		switch (dir) {

		case PLUSX:

			return NEGX;
			break;

		case NEGX:

			return PLUSX;
			break;

		case PLUSY:

			return NEGY;
			break;

		case NEGY:

			return PLUSY;
			break;

		case PLUSZ:

			return NEGZ;
			break;

		case NEGZ:

			return PLUSZ;
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


}