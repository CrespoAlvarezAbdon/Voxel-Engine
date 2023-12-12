/**
* @file vertexArray.h
* @version 1.0
* @date 25/04/2023
* @author Abdon Crespo Alvarez
* @title Vertex array.
* @brief Contains the definition of the vertex array object (VAO) used in the engine.
*/
#ifndef _VOXELENG_VERTEXARRAY_
#define _VOXELENG_VERTEXARRAY_
#include "vertexBuffer.h"
#include "vertexBufferLayout.h"


namespace VoxelEng {

	////////////
	//Classes.//
	////////////

	/**
	* @brief A vertex array is responsible for collecting the data from the used vertex buffers
	* and the format of this data (stored in vertexBufferLayout objects).
	*/
	class vertexArray {

	public:

		// Constructors.

		/**
		* @brief Class constructor that creates and empty vertex buffer with no association
		* with a real graphics API buffer.
		*/
		vertexArray();


		// Modifiers.

		void generate();

		/**
		* @brief Bind the VAO to the graphics API context.
		* WARNING. Must be called in a thread with valid graphics API context.
		*/
		void bind();

		/**
		* @brief Unbind the VAO from the graphics API contex.
		* WARNING. Must be called in a thread with valid graphics API context.
		*/
		void unbind();

		/**
		* @brief Add a vertex buffer layout to the VAO so the GPU can figure out
		* what is the data we sent to it.
		* WARNING. Must be called in a thread with valid graphics API context.
		*/
		void addLayout(const vertexBufferLayout& layout);

		/**
		* @brief Submit vertex data from a vertex buffer object to an VAO which has been prepared for dynamic
		* geometry with a call to vertexBuffer::prepareDynamic(...) done before this one.
		* WARNING. Must be called in a thread with valid graphics API context.
		*/
		void addDynamicBuffer(const vertexBuffer& vb, const vertexBufferLayout& layout, size_t size, const void* data);


		// Destructors.

		/**
		* @brief Class destructor.
		* WARNING. Must be called in a thread with valid graphics API context.
		*/
		~vertexArray();
		
	private:

		GLuint rendererID_;

	};

}

#endif