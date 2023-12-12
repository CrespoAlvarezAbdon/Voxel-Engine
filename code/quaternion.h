/**
* @file quaternion.h
* @version 1.0
* @date 02/12/2023
* @author Abdon Crespo Alvarez
* @title Quaternion.
* @brief Contains the definition of the mathematical concept of quaternion
*/
#ifndef _VOXELENG_QUATERNION_
#define _VOXELENG_QUATERNION_

#include <cmath>
#include "vec.h"

namespace VoxelEng {

	// TODO. METER MOVE CONSTRUCTOR Y MOVE OPERATOR.

	class quaternion {

	public:

		// Constructors.

		quaternion();

		quaternion(float x, float y, float z, float w);

		quaternion(const vec3& v, float w);


		// Observers.

		float norm() const;

		quaternion conjugate() const;

		quaternion inverse() const;

		const vec3& v() const;

		float scalar() const;


		// Operators.

		void operator+=(const quaternion& q2);

		quaternion operator+(const quaternion& q2) const;


		void operator-=(const quaternion& q2);

		quaternion operator-(const quaternion& q2) const;


		void operator*=(const quaternion& q2);

		quaternion operator*(const quaternion& q2) const;


		void operator*=(float value);

		quaternion operator*(float value) const;


		// Modifiers.

		void normalise();

		void normaliseForRotations();


		// Others.

		static vec3 rotateVector(const vec3& v, float angle, vec3 axis);

	private:

		static const float pi,
			               piOver180;

		vec3 v_;
		float s_;

	};

	inline quaternion::quaternion()
	: v_(vec3Zero), s_(0)
	{}

	inline quaternion::quaternion(float x, float y, float z, float s)
	: v_{x, y, z}, s_(s)
	{}

	inline quaternion::quaternion(const vec3& v, float s)
	: v_(v), s_(s)
	{}

	inline float quaternion::norm() const {

		return std::sqrt(s_ * s_ + glm::dot(v_,v_));

	}

	inline quaternion quaternion::conjugate() const {

		return quaternion(v_ * (-1.0f), s_);

	}

	inline const vec3& quaternion::v() const {
	
		return v_;
	
	}

	inline float quaternion::scalar() const {

		return s_;

	}

	inline quaternion quaternion::operator+(const quaternion& q2) const {

		return quaternion(v_ + q2.v_, s_ + q2.s_);

	}

	inline quaternion quaternion::operator-(const quaternion& q2) const {

		return quaternion(v_ - q2.v_, s_ - q2.s_);

	}

	inline quaternion quaternion::operator*(const quaternion& q2) const {

		return quaternion(q2.v_ * s_ + v_ * q2.s_ + glm::cross(v_, q2.v_), s_ * q2.s_ - glm::dot(v_, q2.v_));

	}

	inline quaternion quaternion::operator*(float value) const {

		return quaternion(v_ * value, s_ * value);

	}

}

#endif