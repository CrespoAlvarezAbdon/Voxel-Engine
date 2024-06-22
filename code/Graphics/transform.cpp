#include "transform.h"

#if GRAPHICS_API == OPENGL

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm.hpp>
#include <gtx/vector_angle.hpp>

#endif

namespace VoxelEng {

	void transform::getRotationFromYaxis(vec3& axis, float& angle) const {

		axis = glm::normalize(glm::cross(Yaxis, vec3FixedUp));
		angle = glm::degrees(glm::angle(Yaxis, vec3FixedUp));

	}

}