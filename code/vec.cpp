#include "vec.h"
#include <algorithm>
#include <quaternion.h>
#include <utilities.h>
#include <Utilities/BlockViewDir/blockViewDir.hpp>

namespace glm {

	bool operator<(const ivec3& v1, const ivec3& v2) {

		if (v1.x != v2.x)
			return v1.x < v2.x;
		else if (v1.y != v2.y)
			return v1.y < v2.y;
		else
			return v1.z < v2.z;

	}

}

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

	basicVec4 basicVec4::clampAdd(const basicVec4& v, const basicVec4& min, const basicVec4& max) const {

		return basicVec4(utilities::clampAdd(x, v.x, min.x, max.x), 
			utilities::clampAdd(y, v.y, min.y, max.y),
			utilities::clampAdd(z, v.z, min.z, max.z),
			utilities::clampAdd(w, v.w, min.w, max.w));

	}

	basicVec4& basicVec4::clampAdd(const basicVec4& v, const basicVec4& min, const basicVec4& max) {
	
		x = utilities::clampAdd(x, v.x, min.x, max.x);
		y = utilities::clampAdd(y, v.y, min.y, max.y);
		z = utilities::clampAdd(z, v.z, min.z, max.z);
		w = utilities::clampAdd(w, v.w, min.w, max.w);

		return *this;

	}

	basicUVec4 basicUVec4::clampAdd(const basicUVec4& v, const basicUVec4& min, const basicUVec4& max) const {

		return basicUVec4(utilities::clampAdd(x, v.x, min.x, max.x),
			utilities::clampAdd(y, v.y, min.y, max.y),
			utilities::clampAdd(z, v.z, min.z, max.z),
			utilities::clampAdd(w, v.w, min.w, max.w));

	}

	basicUVec4& basicUVec4::clampAdd(const basicUVec4& v, const basicUVec4& min, const basicUVec4& max) {

		x = utilities::clampAdd(x, v.x, min.x, max.x);
		y = utilities::clampAdd(y, v.y, min.y, max.y);
		z = utilities::clampAdd(z, v.z, min.z, max.z);
		w = utilities::clampAdd(w, v.w, min.w, max.w);

		return *this;

	}

}