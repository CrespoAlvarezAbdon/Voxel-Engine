#include "texture.h"
#include "External code/stb_image.h"


namespace VoxelEng {

	// 'texture' class.

	std::unordered_map<unsigned int, std::pair<int, int>> texture::texturesHW_ = {

		{929, {32, 16}},
		{961, {32, 16}},
		{993, {32, 16}},
		{995, {64, 64}},
		{999, {32, 16}},
		{1001, {32, 16}},
		{1003, {32, 16}},
		{1005, {32, 16}},
		{1007, {32, 16}},
		{1009, {64, 64}},
		{1013, {64, 64}},
		{1017, {32, 16}},
		{1019, {32, 16}},
	
	};
	texture const* texture::blockTextureAtlas_ = nullptr;
	unsigned int texture::blockAtlasResolution_ = 0;
	std::pair<int, int> texture::defaultTextureHW_ = {16, 16};


	texture::texture(const std::string& filepath)
	: rendererID_(0), textureFilepath_(filepath), buffer_(nullptr), width_(0), height_(0), bitsPerPixel_(0) {

		stbi_set_flip_vertically_on_load(1); // What this does is flip the texture because OpenGL expects the texture to begin in a strange point when loading PNG files ¬¬
		buffer_ = stbi_load(filepath.c_str(), &width_, &height_, &bitsPerPixel_, 4); // Last parameter is how many channels we want. We are using RGBA (A stands for Alpha) so we want four channels

		glCreateTextures(GL_TEXTURE_2D, 1, &rendererID_);
		glBindTexture(GL_TEXTURE_2D, rendererID_);

		// Options used when we need to scale down the texture.
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST); 

		// Options used when we need to scale up the texture.
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST); 

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

		// Everything ready. Send the texture to OpenGL. 
		// If the last parameter is nullptr, 
		// we are only allocating space in OpenGL,
		// not providing it any data.
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width_, height_, 0, GL_RGBA, GL_UNSIGNED_BYTE, buffer_);	

		// Clear the local buffer.
		if (buffer_) 
			stbi_image_free(buffer_);

	}

	void texture::bind(unsigned int slot) const {

		glBindTextureUnit(slot, rendererID_);

	}

	void texture::unbind() const {

		glBindTexture(GL_TEXTURE_2D, 0);

	}

	texture::~texture() {

		glDeleteTextures(1, &rendererID_);

	}

}