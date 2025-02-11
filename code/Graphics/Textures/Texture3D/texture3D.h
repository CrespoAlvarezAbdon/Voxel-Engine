#ifndef _VOXELENG_TEXTURE_3D_
#define _VOXELENG_TEXTURE_3D_

#include <Graphics/Textures/texture.h>

namespace VoxelEng {

	class texture3D : public texture {
	
	public:

		// Constructors.

		/**
		* @brief Class constructor.
		* Creates a texture to hold the data of an array of size 'sizeX' x 'sizeY' x 'sizeZ' x 3 elements.
		* The texture will be in RGB format, where for every coordinate in the first three dimensions, three values of 8 bits
		* will be present.
		* @param sizeX Size of the first dimension. Must be greater than 0.
		* @param sizeY Size of the second dimension. Must be greater than 0.
		* @param sizeZ Size of the third dimension. Must be greater than 0.
		* @param data Pointer to the beginning of the array.
		* Cannot be null and the array must be of size 'sizeX' x 'sizeY' x 'sizeZ' x 3 elements.
		*/
		texture3D(unsigned int sizeX, unsigned int sizeY, unsigned int sizeZ);


		// Observers.

		/**
		* @brief Returns the texture third dimension's size.
		*/
		int depth() const;


		// Modifiers.

		/**
		* @brief Set and upload the image's data to the GPU.
		* @param data Pointer to the beginning of the image's data. It must correspond to the beginning
		* of an unsigned char of size image's width * image's height * image's depth * 3. The data will be read in RGB format
		* where each channel has 1 byte.
		*/
		void setAndUploadData(const unsigned char* data);


		// Destructors.

		/**
		* @brief Class destructor.
		*/
		~texture3D();

	private:
		
		int depth_;
	
	};

	inline int texture3D::depth() const {

		return depth_;

	}

	inline void texture3D::setAndUploadData(const unsigned char* data) {

		glTextureSubImage3D(rendererID_, 0, 0, 0, 0, width_, height_, depth_, GL_RGB, GL_UNSIGNED_BYTE, data);

	}

	inline texture3D::~texture3D() {
	
		destroy();
	
	}

}

#endif