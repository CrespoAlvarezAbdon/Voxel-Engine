#include "fustrum.h"

#include "../../game.h"
#include "../../gameWindow.h"

#include <cmath>

#if GRAPHICS_API == OPENGL

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm.hpp>

#endif

namespace VoxelEng {

	fustrum::fustrum(const camera& cam) {
	
		const float aspect = game::getWindow().aspectRatio();
		const float halfVSide = cam.zFar() * tanf(cam.FOV() * 0.5f);
		const float halfHSide = halfVSide * aspect;
		const vec3 frontMultFar = cam.zFar() * cam.viewDirection();

		near_.normal(cam.viewDirection());
		near_.distance(cam.globalPos() + cam.zNear() * cam.viewDirection());

		far_.normal(-cam.viewDirection());
		far_.distance(cam.globalPos() + frontMultFar);

		//right_.normal(glm::cross(frontMultFar - cam.Zaxis() * halfHSide, cam.Yaxis())); EL PAVO LO PONIA ASÍ PERO NO ME PUTO FÍO LA VERDAD. LOS DE ABAJO TAMBIÉN LOS HE CAMBIADO YO.
		right_.normal(glm::cross(frontMultFar + cam.Zaxis() * halfHSide, cam.Yaxis()));
		right_.distance(cam.globalPos());

		left_.normal(glm::cross(cam.Yaxis(), frontMultFar + cam.Zaxis() * halfHSide));
		left_.distance(cam.globalPos());

		top_.normal(glm::cross(cam.Zaxis(), frontMultFar + cam.Yaxis() * halfVSide));
		top_.distance(cam.globalPos());

		bottom_.normal(glm::cross(frontMultFar - cam.Yaxis() * halfVSide, cam.Zaxis()));
		bottom_.distance(cam.globalPos());
	
	}

}