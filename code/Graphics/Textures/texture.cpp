#include "texture.h"
#include "../framebuffer.h"
#include "../../External code/stb_image.h"
#include <Utilities/Logger/logger.h>

namespace VoxelEng {

	// 'texture' class.

	// TODO. THIS WILL BE DYNAMICALLY CREATED WITH THE DYNAMIC ATLAS.
	std::unordered_map<unsigned int, std::pair<int, int>> texture::texturesWH_ = {

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
	std::pair<int, int> texture::defaultTextureWH_ = {16, 16};
	const int texture::maxColorAttachmentIndex_ = 15;


	texture::texture(unsigned int width, unsigned int height, textureType type, framebuffer* bufferToAttachTo, int colorAttachmentIndex)
	: rendererID_(0), textureFilepath_(""), buffer_(nullptr), width_(width), height_(height), bitsPerPixel_(0), type_(type) {
	
		if (bufferToAttachTo && type != textureType::DEPTH_AND_STENCIL && (colorAttachmentIndex < -1 || colorAttachmentIndex > maxColorAttachmentIndex_))
			logger::errorLog("The value provided for the 'colorAttachmentIndex' parameter (" + std::to_string(colorAttachmentIndex) + ") is not valid.");

		switch (type) {
		
			case textureType::NONE:
				logger::errorLog("Texture type cannot be NONE in constructor");
				break;

			case textureType::COLOR:

				glCreateTextures(GL_TEXTURE_2D, 1, &rendererID_); // POSIBLE ERROR HAY QUE USAR glGenTextures(1, &rendererID_);
				glBindTexture(GL_TEXTURE_2D, rendererID_);

				// Options used when we need to scale down the texture.
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

				// Options used when we need to scale up the texture.
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST); // POSIBLE ERROR POR NO PONER GL_LINEAR???

				// Everything ready. Send the texture to OpenGL. 
				// If the last parameter is nullptr, 
				// we are only allocating space in OpenGL,
				// not providing it any data.
				glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width_, height_, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr); // POSIBLE ERROR ESTO VA A ANTES QUE TEXPARAMETERI???
				// POSIBLE ERROR GL_RGB EN VEZ DE GL_RGBA???

				glBindTexture(GL_TEXTURE_2D, 0);

				if (bufferToAttachTo) {
			
					if (colorAttachmentIndex == -1)
						glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + bufferToAttachTo->nAttachments(type), GL_TEXTURE_2D, rendererID_, 0);
					else
						glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + colorAttachmentIndex, GL_TEXTURE_2D, rendererID_, 0);
			
				}
				
				break;

			case textureType::DEPTH_AND_STENCIL:

				glGenRenderbuffers(1, &rendererID_);
				glBindRenderbuffer(GL_RENDERBUFFER, rendererID_);
				glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height); // POSSIBLE ERROR IGUAL HAY QUE HACER DEPTHTEXTURE COMO EN LEARNOPENGL???
				glBindRenderbuffer(GL_RENDERBUFFER, 0);

				if (bufferToAttachTo)
					glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rendererID_);

				break;

			case textureType::REVEAL:

				glCreateTextures(GL_TEXTURE_2D, 1, &rendererID_); // POSIBLE ERROR HAY QUE USAR glGenTextures(1, &rendererID_);
				glBindTexture(GL_TEXTURE_2D, rendererID_);

				// Options used when we need to scale down the texture.
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

				// Options used when we need to scale up the texture.
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST); // POSIBLE ERROR POR NO PONER GL_LINEAR???

				// Everything ready. Send the texture to OpenGL. 
				// If the last parameter is nullptr, 
				// we are only allocating space in OpenGL,
				// not providing it any data.
				glTexImage2D(GL_TEXTURE_2D, 0, GL_R8, width_, height_, 0, GL_RED, GL_FLOAT, nullptr); // POSIBLE ERROR ESTO VA A ANTES QUE TEXPARAMETERI???
				// POSIBLE ERROR GL_RGB EN VEZ DE GL_RGBA???

				glBindTexture(GL_TEXTURE_2D, 0);

				if (bufferToAttachTo) {

					if (colorAttachmentIndex == -1)
						glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + bufferToAttachTo->nAttachments(type), GL_TEXTURE_2D, rendererID_, 0);
					else
						glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + colorAttachmentIndex, GL_TEXTURE_2D, rendererID_, 0);

				}

				break;

			case textureType::IMAGE:
				logger::errorLog("It is not supported to attach Image type textures into a framebuffer");
				break;

			default:
				logger::errorLog("Unsupported texture type " + std::to_string(static_cast<int>(type)));
				break;
		
		}

	}

	texture::texture(const std::string& filepath)
	: rendererID_(0), textureFilepath_(filepath), buffer_(nullptr), width_(0), height_(0), bitsPerPixel_(0), type_(textureType::IMAGE) {

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
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width_, height_, 0, GL_RGBA, GL_UNSIGNED_BYTE, buffer_);	

		// Clear the local buffer.
		if (buffer_) 
			stbi_image_free(buffer_);

	}

	void texture::bind(unsigned int slot) const {

		glActiveTexture(GL_TEXTURE0 + slot); // POSIBLE ERROR? IGUAL HAY QUE HACER SWI O INCLUSO NO USAR ESTO?? ESTO BASICAMENTE DICE QUE CAMBIES EL SLOT DE TEXTURA ACTIVA.
		glBindTextureUnit(slot, rendererID_);

	}

	void texture::unbind() const {

		glBindTexture(GL_TEXTURE_2D, 0);

	}

	texture::~texture() {

		glDeleteTextures(1, &rendererID_);

	}

}