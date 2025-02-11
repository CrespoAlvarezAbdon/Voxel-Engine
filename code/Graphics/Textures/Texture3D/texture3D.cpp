#include "texture3D.h"

#include <Graphics/graphics.h>

namespace VoxelEng {

	texture3D::texture3D(unsigned int sizeX, unsigned int sizeY, unsigned int sizeZ)
	: texture(sizeX, sizeY, 8, textureType::ARRAY), depth_(sizeZ)
	{
		glCreateTextures(GL_TEXTURE_3D, 1, &rendererID_);
		glBindTexture(GL_TEXTURE_3D, rendererID_);

		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

		glTexImage3D(GL_TEXTURE_3D, 0, GL_RGB8, width_, height_, depth_, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);

		glBindTexture(GL_TEXTURE_3D, 0);

	}

}