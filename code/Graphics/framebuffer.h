#ifndef _VOXELENG_FRAMEBUFFER_
#define _VOXELENG_FRAMEBUFFER_

#include <initializer_list>
#include <unordered_map>
#include <memory>
#include <vector>
#include "../vec.h"
#include "../Graphics/graphics.h"
#include "../Graphics/Textures/texture.h"

namespace VoxelEng {

	/**
	* @brief Class used to represent a buffer with size equal to the display screen
	* that can hold multiple buffers with different purposes as attachments.
	*/
	class framebuffer {
	
	public:

		// Constructors.

		/**
		* @brief Default class constructor.
		* Initializes the framebuffer with all its specified buffers.
		*/
		framebuffer(unsigned int width, unsigned int height, std::initializer_list<textureType> buffers);


		// Observers.

		/**
		* @brief Get the total number of attachments.
		*/
		unsigned int nTotalAttachments() const;

		/**
		* @brief Get the number of attachments of the specified type.
		*/
		unsigned int nAttachments(textureType type) const;

		/**
		* @brief Returns the framebuffer's texture.
		* For example, getTexture(framebufferAttachment::COLOR, 3) will get the third
		* color buffer that was attached to the framebuffer.
		* Throws exception if the specified texture does not exist attached to the framebuffer at the given position.
		*/
		const texture& getTexture(textureType type, unsigned int i) const;


		// Modifiers.

		/**
		* @brief Bind the framebuffer so that all render commands affect its buffer instead.
		* NOTE. For these render commands to properly affect this framebuffer, Framebuffer::readyToBeDrawn() must return
		* true first.
		*/
		void bind();

		/**
		* @brief Unbind the currently bound framebuffer and bind the default framebuffer. The last one is the one that will
		* be used to draw on screen.
		*/
		void unbind();

		/**
		* @brief Returns the framebuffer's texture.
		* For example, getTexture(framebufferAttachment::COLOR, 3) will get the third
		* color buffer that was attached to the framebuffer.
		* Throws exception if the specified texture does not exist attached to the framebuffer at the given position.
		*/
		texture& getTexture(textureType type, unsigned int i);

		/**
		* @brief Push at the end of the framebuffer the given texture.
		*/
		void pushBack(texture& texture);

		/**
		* @brief Clear the attached textures with the specified clear colors.
		*/
		void clearTextures(std::initializer_list<vec4> clearColors = {});

		/**
		* @brief Clear all the attached textures to the graphics API selected clear color.
		*/
		void clearAllTextures();


		// Destructors.

		/**
		* @brief Class destructor.
		*/
		~framebuffer();

	private:

		static GLenum supportedColorBuffers_[16];

		unsigned int ID_;
		unsigned int nTotalAttachments_;
		std::unordered_map<textureType, std::vector<std::shared_ptr<texture>>> attachedTextures_;
	
	};

	inline unsigned int framebuffer::nTotalAttachments() const {
	
		return nTotalAttachments_;
	
	}

	inline unsigned int framebuffer::nAttachments(textureType type) const {

		return attachedTextures_.contains(type) ? attachedTextures_.at(type).size() : 0;
	
	}

	inline void framebuffer::bind() {

		glBindFramebuffer(GL_FRAMEBUFFER, ID_);

	}

	inline void framebuffer::unbind() {

		glBindFramebuffer(GL_FRAMEBUFFER, 0);

	}

}

#endif