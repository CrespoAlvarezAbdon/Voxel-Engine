#include "quaternion.h"
#include <cmath>

#if GRAPHICS_API == OPENGL

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm.hpp>

#endif

namespace VoxelEng {

	// 'quaternion' class.

	const float quaternion::pi = 3.141592f,
				quaternion::piOver180 = pi / 180.0f;


	quaternion quaternion::inverse() const {

		quaternion c = conjugate();
		float n = norm();
		n *= n;
		n = 1 / n;

		return quaternion(c.v_ * n, c.s_ * n);

	}

	void quaternion::operator+=(const quaternion& q2) {
	
		v_ += q2.v_;
		s_ += q2.s_;
	
	}

	void quaternion::operator-=(const quaternion& q2) {

		v_ -= q2.v_;
		s_ -= q2.s_;

	}

	void quaternion::operator*=(const quaternion& q2) {

		v_ = q2.v_ * s_ + v_ * q2.s_ + glm::cross(v_, q2.v_);
		s_ = s_ * q2.s_ - glm::dot(v_, q2.v_);

	}

	void quaternion::operator*=(float value) {

		v_ *= value;
		s_ *= value;

	}

	void quaternion::normalise() {

		float n = norm();

		// Only normalise if necessary.
		if (n != 0) {

			float oneOverN = 1.0f / n;

			v_ *= oneOverN;
			s_ *= oneOverN;

		}

	}

	void quaternion::normaliseForRotations() {

		float angle = s_ * piOver180; // Assuming s_ is in degrees.
		v_ = glm::normalize(v_);
		s_ = std::cosf(angle * 0.5);
		v_ = v_ * std::sinf(angle * 0.5);

	}

	vec3 quaternion::rotateVector(const vec3& v, float angle, vec3 axis) {

		quaternion pure(v, 0);
		quaternion q(axis, angle);

		axis = glm::normalize(axis);

		q.normaliseForRotations();

		quaternion qInv = q.inverse();
		quaternion rotatedQ = q * pure * qInv;

		return rotatedQ.v_;

	}

}