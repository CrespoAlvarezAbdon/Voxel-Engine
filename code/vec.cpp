#include "vec.h"
#include "quaternion.h"
#include <Utilities/BlockViewDir/blockViewDir.hpp>

namespace VoxelEng {

	vec3 operator+(const vec3& v, blockViewDir viewDir) {

		switch (viewDir) {
		case blockViewDir::NONE:
			throw std::runtime_error("NONE cannot be a block view direction to add to a vec3");
		case blockViewDir::PLUSX:
			return vec3{ v.x + 1, v.y, v.z };
		case blockViewDir::NEGX:
			return vec3{ v.x - 1, v.y, v.z };
		case blockViewDir::PLUSY:
			return vec3{ v.x , v.y + 1, v.z };
		case blockViewDir::NEGY:
			return vec3{ v.x , v.y - 1, v.z };
		case blockViewDir::PLUSZ:
			return vec3{ v.x , v.y, v.z + 1 };
		case blockViewDir::NEGZ:
			return vec3{ v.x , v.y, v.z - 1 };
		default:
			throw std::runtime_error(
				"Unsupported block view direction " + std::to_string(static_cast<int>(viewDir)) + " cannot be added to vec3");
		}

	}

}