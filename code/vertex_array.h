#ifndef _VERTEXARRAY_
#define _VERTEXARRAY_
#include "vertex_buffer.h"
#include "vertex_buffer_layout.h"


namespace VoxelEng {

	////////////
	//Classes.//
	////////////

	/*
	Represents a vertex array object (VAO) from OpenGL.
	See the corresponding official documentation for more info about VAOs.
	*/
	class vertexArray {

	public:

		// Constructors.

		/*
		WARNING. Must be called in a thread with valid OpenGL context.
		*/
		vertexArray();


		// Modifiers.

		/*
		Bind the VAO.
		WARNING. Must be called in a thread with valid OpenGL context.
		*/
		void bind() const;

		/*
		Unbind the VAO.
		WARNING. Must be called in a thread with valid OpenGL context.
		*/
		void unbind() const;

		/*
		Add a vertex buffer layout to the VAO so the GPU can figure out
		what is the data we sent to it.
		WARNING. Must be called in a thread with valid OpenGL context.
		*/
		void addLayout(const vertexBufferLayout& layout);

		/*
		Submit vertex data from a vertex buffer object to an VAO which has been prepared for dynamic
		geometry with a call to vertexBuffer::preprareDynamic(...) done before this one.
		WARNING. Must be called in a thread with valid OpenGL context.
		*/
		void addDynamicBuffer(const vertexBuffer& vb, const vertexBufferLayout& layout, size_t size, const void* data);


		// Destructors.

		/*
		WARNING. Must be called in a thread with valid OpenGL context.
		*/
		~vertexArray();

	private:

		unsigned int rendererID_;

	};

}

#endif