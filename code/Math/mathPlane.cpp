#include "mathPlane.h"

namespace VoxelEng {

	namespace Math {

		void plane::point(const vec3& newPoint) {

			point_ = newPoint;

			updateDotPointNormal();
			
		}

		void plane::normal(const vec3& newValue) {

			normal_.x = newValue.x;
			normal_.y = newValue.y;
			normal_.z = newValue.z;
			normal_ = glm::normalize(normal_);

			updateDotPointNormal();

		}

	}

}