#include "Framebuffer.h"
#include <Graphics/graphics.h>
#include <Utilities/Logger/logger.h>

#if GRAPHICS_API == OPENGL

#include <GL/glew.h>

#endif

namespace VoxelEng {

	GLenum framebuffer::supportedColorBuffers_[16] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2,
		GL_COLOR_ATTACHMENT3, GL_COLOR_ATTACHMENT4, GL_COLOR_ATTACHMENT5,
		GL_COLOR_ATTACHMENT6, GL_COLOR_ATTACHMENT7, GL_COLOR_ATTACHMENT8,
		GL_COLOR_ATTACHMENT9, GL_COLOR_ATTACHMENT10, GL_COLOR_ATTACHMENT11,
		GL_COLOR_ATTACHMENT12 ,GL_COLOR_ATTACHMENT13, GL_COLOR_ATTACHMENT14, GL_COLOR_ATTACHMENT15};

	framebuffer::framebuffer(unsigned int width, unsigned int height, std::initializer_list<textureType> attachments)
	: ID_(0)
	{
	
		// Create framebuffer.
		glGenFramebuffers(1, &ID_);

		bind();
		
		// NEW
		textureType type = textureType::NONE;
		for (std::initializer_list<textureType>::const_iterator it = attachments.begin(); it != attachments.end(); it++) {
		
			type = *it;

			attachedTextures_[type].push_back(std::make_shared<texture>(width, height, type, this));

		}
		nTotalAttachments_ = attachments.size();

		// Tell OpenGL how many buffers will be drawn for this framebuffer. A.K.A how many output targets the fragment shader will have??
		if (unsigned int nColorAttachments = nAttachments(textureType::COLOR))
			glDrawBuffers(nColorAttachments, supportedColorBuffers_);

		// OLD

		// Create and attach texture (color) buffer.
		//texture_ = new texture(width, height);
		//glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture_->rendererID(), 0);

		// Create and attach depth and stencil buffers.
		//unsigned int rbID;
		//glGenRenderbuffers(1, &rbID);
		//glBindRenderbuffer(GL_RENDERBUFFER, rbID);
		//glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height); // POSSIBLE ERROR IGUAL HAY QUE HACER DEPTHTEXTURE COMO EN LEARNOPENGL???
		//glBindRenderbuffer(GL_RENDERBUFFER, 0);

		//glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbID);

		if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
			logger::errorLog("Error while creating framebuffer");

		unbind();

	}

	const std::shared_ptr<texture>& framebuffer::getTexture(textureType type, unsigned int i) const {
	
		if (attachedTextures_.contains(type) && attachedTextures_.at(type).size() > i)
			return attachedTextures_.at(type)[i];
		else
			logger::errorLog("The specified attached texture with type " + std::to_string((int)type) + " at index " + std::to_string(i) + " does not exist");
	
	}

	std::shared_ptr<texture>& framebuffer::getTexture(textureType type, unsigned int i) {

		if (attachedTextures_.contains(type) && attachedTextures_.at(type).size() > i)
			return attachedTextures_.at(type)[i];
		else
			logger::errorLog("The specified attached texture with type " + std::to_string((int)type) + " at index " + std::to_string(i) + " does not exist");

	}

	void framebuffer::pushBack(std::shared_ptr<texture>& t) {
	
		textureType type = t->type();

		switch (type) {
		
			case textureType::NONE:
				logger::errorLog("The specified textureType cannot be NONE");
				break;

			case textureType::COLOR:
			case textureType::REVEAL:
				glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + attachedTextures_[type].size(), GL_TEXTURE_2D, t->rendererID(), 0);
				break;

			case textureType::DEPTH_AND_STENCIL: // TODO. PONER EXCEPCION DE QUE SOLO PUEDE HABER UN DEPTH AND STENCIL ASI QUE ESTE SOBRESCRIBE AL ANTERIOR EN ESTE CASO LUEGO MIRA COMO MANEJAR ESTE Y LOS OTROS CASOS DE INSERCION Y DEMÁS.
				glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, t->rendererID());
				break;

			case textureType::IMAGE:
				logger::errorLog("It is not supported to attach Image type textures into a framebuffer");
				break;

			default:
				logger::errorLog("Unsupported texture type " + std::to_string(static_cast<int>(type)));
				break;
		}

		attachedTextures_[type].emplace_back(t);
	
	}

	void framebuffer::clearTextures(std::initializer_list<vec4> clearColors) {
	
		if (nTotalAttachments_ == 1) {
		
			if (clearColors.size() != 0)
				logger::errorLog("When the total number of attachments in a framebuffer is 1, clearColors is not needed");
			else
				glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
		
		}
		else {
		
			if (clearColors.size() == nTotalAttachments_) {

				std::vector<unsigned int> graphicsAPIValues;
				for (std::unordered_map<textureType, std::vector<std::shared_ptr<texture>>>::const_iterator it = attachedTextures_.cbegin();
					it != attachedTextures_.cend(); it++) {

					int i = 0;
					std::initializer_list<vec4>::const_iterator clearColorIt = clearColors.begin();
					for (std::vector<std::shared_ptr<texture>>::const_iterator itType = it->second.cbegin(); itType != it->second.cend(); itType++) {

						graphics::textureTypeToAPITextureType(it->first, graphicsAPIValues);

						for (int j = 0; j < graphicsAPIValues.size(); j++) // TODO. SUSTITUIR TODA ESTA HISTORIA POR UN SIMPLE GLCLEARBUFFER COMPLETITO Y YA???
							glClearBufferfv(graphicsAPIValues[j], i, &(*clearColorIt)[0]); // WARNING. &(*clearColorIt)[0] ASSUMES THAT VEC4 LIES ITS COMPONENTS CONTINOUSLY IN MEMORY.

						i++;
						clearColorIt++;

					}

				}

			}
			else
				logger::errorLog("The number of clear colors to use and the total number of attachments for the framebuffer are not equal");

		}
	
	}

	void framebuffer::clearAllTextures() {
	
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
	
	}

	framebuffer::~framebuffer() {

		glDeleteFramebuffers(1, &ID_);
	
	}

}