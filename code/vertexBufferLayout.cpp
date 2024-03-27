#include "vertexBufferLayout.h"


namespace VoxelEng {

	// Non-member functions.

	unsigned int graphicsAPISizeOf(unsigned int type)
	{

		switch (type)
		{

			case GL_FLOAT: return 4;
			case GL_UNSIGNED_INT: return 4;
			case GL_UNSIGNED_BYTE: return 1;
			case GL_BYTE: return 1;
			case GL_INT_2_10_10_10_REV: return 4;
			default: return 0;

		}

	}


	// 'vertexBufferLayout' class.

	template <>
	void vertexBufferLayout::push<GLfloat>(unsigned int count)
	{

		elements_.push_back({ GL_FLOAT, count, true }); // POSIBLE ERROR QUE ISNORMALIZED DEBER�A ESTAR A FALSE PARA SCREEN QUAD?

		stride_ += count * graphicsAPISizeOf(GL_FLOAT);

	}

	template <>
	void vertexBufferLayout::push<unsigned int>(unsigned int count)
	{

		elements_.push_back({ GL_UNSIGNED_INT, count, false });

		stride_ += count * graphicsAPISizeOf(GL_UNSIGNED_INT);

	}

	template <>
	void vertexBufferLayout::push<unsigned char>(unsigned int count)
	{

		elements_.push_back({ GL_UNSIGNED_BYTE, count, true });

		stride_ += count * graphicsAPISizeOf(GL_UNSIGNED_BYTE);

	}

	template <>
	void vertexBufferLayout::push<GLbyte>(unsigned int count)
	{

		elements_.push_back({ GL_BYTE, count, false });

		stride_ += count * graphicsAPISizeOf(GL_BYTE);

	}

	template <>
	void vertexBufferLayout::push<GLint>(unsigned int count)
	{

		elements_.push_back({ GL_INT_2_10_10_10_REV, count*4, false });

		stride_ += count * graphicsAPISizeOf(GL_INT_2_10_10_10_REV);

	}

}