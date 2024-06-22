#include "frustum.h"

#include "../../game.h"
#include "../../gameWindow.h"
#include "../../entity.h"
#include "../../Entities/plane.h"

#include <cmath>

#if GRAPHICS_API == OPENGL

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm.hpp>

#endif

namespace VoxelEng {

	frustum::frustum(const camera& cam) {
	
		const float aspect = game::getWindow().aspectRatio();
		const float halfVSide = cam.zFar() * 0.7f;
		const float halfHSide = halfVSide * aspect;
		const vec3 frontMultFar = cam.zFar() * cam.viewDirection();
		const vec3 frustumPos = cam.globalPos() - 16.0f * cam.viewDirection();

		near_.normal(cam.viewDirection());
		near_.point(frustumPos);

		far_.normal(-cam.viewDirection());
		far_.point(cam.globalPos() + frontMultFar);

		right_.normal(frontMultFar - cam.Zaxis() * halfHSide);
		right_.point(frustumPos);

		left_.normal(frontMultFar + cam.Zaxis() * halfHSide);
		left_.point(frustumPos);

		top_.normal(frontMultFar + cam.Yaxis() * halfVSide);
		top_.point(frustumPos);

		bottom_.normal(frontMultFar - cam.Yaxis() * halfVSide);
		bottom_.point(frustumPos);
	
	}

	void frustum::spawnDebugPlanes() const {
	
		const vec3& nearPos = near_.point();
		const vec3& nearNormal = near_.normal();
		const vec3& farPos = far_.point();
		const vec3& farNormal = far_.normal();
		const vec3& rightPos = right_.point();
		const vec3& rightNormal = right_.normal();
		const vec3& leftPos = left_.point();
		const vec3& leftNormal = left_.normal();
		const vec3& topPos = top_.point();
		const vec3& topNormal = top_.normal();
		const vec3& bottomPos = bottom_.point();
		const vec3& bottomNormal = bottom_.normal();

		entityManager::spawnEntity<plane>(nearPos.x, nearPos.y, nearPos.z, nearNormal.x, nearNormal.y, nearNormal.z, 100.0f, 100.0f, 100.f);
		entityManager::spawnEntity<plane>(farPos.x, farPos.y, farPos.z, farNormal.x, farNormal.y, farNormal.z, 100.0f, 100.0f, 100.f);
		entityManager::spawnEntity<plane>(rightPos.x, rightPos.y, rightPos.z, rightNormal.x, rightNormal.y, rightNormal.z, 100.0f, 100.0f, 100.f);
		entityManager::spawnEntity<plane>(leftPos.x, leftPos.y, leftPos.z, leftNormal.x, leftNormal.y, leftNormal.z, 100.0f, 100.0f, 100.f);
		entityManager::spawnEntity<plane>(topPos.x, topPos.y, topPos.z, topNormal.x, topNormal.y, topNormal.z, 100.0f, 100.0f, 100.f);
		entityManager::spawnEntity<plane>(bottomPos.x, bottomPos.y, bottomPos.z, bottomNormal.x, bottomNormal.y, bottomNormal.z, 100.0f, 100.0f, 100.f);
	
	}

	bool frustum::isInside(const vec3& point) const {
	
		return far_.isPointForward(point) &&
			right_.isPointForward(point) && left_.isPointForward(point);
	
	}

}