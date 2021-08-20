#include <string>
#include <GL/glew.h>
#include "texture.h"
#include "External code/stb_image.h"
using namespace std;

Texture::Texture(const string& filepath) : renderer_ID_(0), texture_filepath_(filepath), local_buffer_(nullptr), width_(0), height_(0), bits_per_pixel(0)
{

	stbi_set_flip_vertically_on_load(1); // What this does is flip the texture because OpenGL expects the texture to begin in a strange point when loading PNG files ¬¬
	local_buffer_ = stbi_load(filepath.c_str(), &width_, &height_, &bits_per_pixel, 4); // Last parameter is how many channels we want. We are using RGBA (A stands for Alpha) so we want four channels

	glCreateTextures(GL_TEXTURE_2D, 1, &renderer_ID_);
	glBindTexture(GL_TEXTURE_2D, renderer_ID_);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST); // Options used when we need to scale down the texture
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST); // Options used when we need to scale up the texture
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width_, height_, 0, GL_RGBA, GL_UNSIGNED_BYTE, local_buffer_);	// Everything ready. Send all to OpenGL. If the last parameter is nullptr, we are only allocating space in OpenGL
																											// not providing it any data.

	if (local_buffer_) // If 'local_buffer_' contains data
		stbi_image_free(local_buffer_);

}

void Texture::bind(unsigned int slot) const
{

	glBindTextureUnit(slot, renderer_ID_);

}

void Texture::unbind() const
{

	glBindTexture(GL_TEXTURE_2D, 0);

}

Texture::~Texture()
{

	glDeleteTextures(1, &renderer_ID_);

}