#include "vertex_buffer_layout.h"

unsigned int OpenGL_size_of(unsigned int type)
{

	switch (type)
	{

		case GL_FLOAT: return 4;
		case GL_UNSIGNED_INT: return 4;
		case GL_UNSIGNED_BYTE: return 1;
		case GL_BYTE: return 1;
		default: return 0;

	}

}

template <>
void vertexBufferLayout::push<GLfloat>(unsigned int count)
{

	elements_.push_back({ GL_FLOAT, count, true });

	stride_ += count * OpenGL_size_of(GL_FLOAT);

}

template <>
void vertexBufferLayout::push<unsigned int>(unsigned int count)
{

	elements_.push_back({ GL_UNSIGNED_INT, count, false });

	stride_ += count * OpenGL_size_of(GL_UNSIGNED_INT);

}

template <>
void vertexBufferLayout::push<unsigned char>(unsigned int count)
{

	elements_.push_back({ GL_UNSIGNED_BYTE, count, true });

	stride_ += count * OpenGL_size_of(GL_UNSIGNED_BYTE);

}

template <>
void vertexBufferLayout::push<GLbyte>(unsigned int count)
{

	elements_.push_back({ GL_BYTE, count, false });

	stride_ += count * OpenGL_size_of(GL_BYTE);

}

template <>
void vertexBufferLayout::push<GLint>(unsigned int count)
{

	elements_.push_back({ GL_INT, count, false });

	stride_ += count * OpenGL_size_of(GL_INT);

}