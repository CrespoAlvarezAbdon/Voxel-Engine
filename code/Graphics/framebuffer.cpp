#include "Framebuffer.h"

#include "../logger.h"

#if GRAPHICS_API == OPENGL

#include <GL/glew.h>

#endif

namespace VoxelEng {

	framebuffer::framebuffer(unsigned int width, unsigned int height)
	: ID_(0),
	  texture_(nullptr)
	{
	
		// Create framebuffer.
		glGenFramebuffers(1, &ID_);

		bind();

		// Create and attach texture (color) buffer.
		texture_ = new texture(width, height);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture_->rendererID(), 0);

		// Create and attach depth and stencil buffers.
		unsigned int rbID;
		glGenRenderbuffers(1, &rbID);
		glBindRenderbuffer(GL_RENDERBUFFER, rbID);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
		glBindRenderbuffer(GL_RENDERBUFFER, 0);

		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbID);

		if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
			logger::errorLog("Error while creating framebuffer");

		unbind();

	}

	void framebuffer::bind() {
	
		glBindFramebuffer(GL_FRAMEBUFFER, ID_);
	
	}

	void framebuffer::unbind() {
	
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		
	}

	texture* framebuffer::colorBuffer() {
	
		return texture_;
	
	}

	framebuffer::~framebuffer() {
	
		glDeleteFramebuffers(1, &ID_);
	
	}

}