/**
* @file texture.h
* @version 1.0
* @date 25/04/2023
* @author Abdon Crespo Alvarez
* @title Texture.
* @brief Contains the declaration of the 'texture' class.
*/
#ifndef _VOXELENG_TEXTURE_
#define _VOXELENG_TEXTURE_

#include <string>
#include <unordered_map>
#include <utility>

#if GRAPHICS_API == OPENGL

#include <GL/glew.h>

#endif


namespace VoxelEng {

	////////////
	//Classes.//
	////////////

	/**
	* @brief Represents a texture, a 2D image usually applied to vertex data
	* in order to give their models an alternative to using solid colors
	* to draw them.
	*/
	class texture {

	public:

		// Constructors.

		/**
		* @brief Class constructor.
		* WARNING. Must be called in a thread with valid graphics API context.
		*/
		texture(const std::string& filepath);


		// Observers.

		/**
		* @brief Returns the texture's width.
		*/
		int width() const;

		/**
		* @brief Returns the texture's width.
		*/
		int height() const;

		/**
		* @brief Returns the texture's ID used by the graphics API to identify it.
		*/
		GLuint rendererID() const;

		/**
		* @brief Returns the currently used block texture atlas.
		*/
		static const texture* blockTextureAtlas();

		/**
		* @brief Returns the currently used block texture atlas' resolution per block.
		*/
		static int blockTextureAtlasResolution();

		/**
		* @brief Returns the width and height of the specified texture with the 'textureID'.
		*/
		static const std::pair<int, int>& getTextureWH(unsigned int textureID);


		// Modifiers.

		/**
		* @brief Set 'blockTextureAtlas' as the block texture atlas.
		*/
		static void setBlockAtlas(const texture& blockTextureAtlas);

		/**
		* @brief Set the block texture atlas' resolution per block.
		* The resolution will always make block textures be square.
		* That is, if you execute texture::blockAtlasResolution() = 32, now
		* the block textures will have a resolution of 32x32 pixels.
		* WARNING. You must set the resolution at least once before the method
		* chunkManager::manageChunks starts executing in the chunk management thread.
		*/
		static void setBlockAtlasResolution(int resolution);


		/**
		* @brief Bind the texture to an graphics API texture slot (slot 0 by default).
		* WARNING. Must be called in a thread with valid graphics API context.
		*/
		void bind(unsigned int slot = 0) const;

		/**
		* @brief Unbind the texture.
		* WARNING. Must be called in a thread with valid graphics API context.
		*/
		void unbind() const;


		// Destructors.

		/**
		* @brief Class destructor.
		* WARNING. Must be called in a thread with valid graphics API context.
		*/
		~texture();

	private:

		static texture const* blockTextureAtlas_;
		static unsigned int blockAtlasResolution_;
		static std::unordered_map<unsigned int, std::pair<int, int>> texturesWH_;
		static std::pair<int, int> defaultTextureWH_;

		GLuint rendererID_;
		std::string textureFilepath_; 
		// Local buffer to store the texture data when loading it from disk.
		unsigned char* buffer_;
		int width_,
			height_,
			bitsPerPixel_;

	};

	inline int texture::width() const {

		return width_;

	}

	inline int texture::height() const {

		return height_;

	}

	inline GLuint texture::rendererID() const {

		return rendererID_;

	}

	inline const texture* texture::blockTextureAtlas() {

		return blockTextureAtlas_;

	}

	inline int texture::blockTextureAtlasResolution() {

		return blockAtlasResolution_;

	}

	inline const std::pair<int, int>& texture::getTextureWH(unsigned int textureID) {

		return texturesWH_.find(textureID) == texturesWH_.cend() ? defaultTextureWH_ : texturesWH_[textureID];

	}

	inline void texture::setBlockAtlas(const texture& blockTextureAtlas) {

		blockTextureAtlas_ = &blockTextureAtlas;

	}

	inline void texture::setBlockAtlasResolution(int resolution) {

		blockAtlasResolution_ = resolution;

	}

}

#endif